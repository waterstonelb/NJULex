#include <iostream>
#include <string>
#include <stack>
#include <set>
#include <map>
#include <queue>
#include <algorithm>
#include <fstream>
using namespace std;

//可支持的char
string STR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const int STRLEN = 62;
//声明结构体
stack<int> workStack;
struct NFAstate
{
    vector<int> a[STRLEN], e;
    bool f = 0;
};
struct DFAstate
{
    int a[STRLEN] = {-1};
    bool f = 0;
};
struct LEX
{
    string token;
    string yyval;
    string value;
};
//实例化NFA与DFA两个初始状态
struct NFAstate NFA_init;
struct DFAstate DFA_init;
///工具函数
//1.字符串分割函数
void SplitString(const string &, vector<string> &, const string &);
//2.优先级比较函数
static int getPriority(char);
//3.输入普通字符时生成NFA状态
void getNormal(int, vector<NFAstate> &);
//4.输入选择符'|'时生成NFA状态
void getSelect(vector<NFAstate> &);
//5.输入连接符'.'时生成NFA状态
void getConnect(vector<NFAstate> &);
//6.输入重复符'*'时生成NFA状态
void getRepeat(vector<NFAstate> &);
//7.求epsilon闭包
void getClosure(int, set<int> &, vector<NFAstate> &);
//8.NFA转DFA时求状态转化集
set<int> state_change(int, set<int> &, vector<NFAstate> &);
//9.解析每一个字符串，返回是否符合当前DFA词法
bool parsingStr(const string, int, vector<DFAstate> &);

///过程函数
//1.re转标准re
string reToStandardRE(string);
//2.re中缀转后缀
string reToPostfix(string);
//3.后缀转NFA
void postfixToNFA(string, vector<NFAstate> &);
//4.NFA转DFA
void NFAtoDFA(vector<NFAstate> &, vector<DFAstate> &, int);
//5.最小化DFA
void minizeDFA(vector<DFAstate> &, vector<DFAstate> &);

int main()
{
    //生成最小状态DNF集合
    ifstream inpLex("resources.txt");
    string lex;
    vector<vector<DFAstate>> M_DFA;
    vector<LEX> LEXs;
    int judge = 0;
    while (inpLex >> lex)
    {
        try
        {
            //对resources中的每一项进行分割
            vector<string> v;
            if (judge++ % 2)
            {
                vector<NFAstate> NFA;
                vector<DFAstate> DFA;
                vector<DFAstate> MiniDFA;
                string stand = reToStandardRE(lex);
                //cout << stand << endl;
                string postfix = reToPostfix(stand);
                //cout << postfix << endl;
                postfixToNFA(postfix, NFA);
                int final_state = workStack.top();
                workStack.pop();
                int start_state = workStack.top();
                workStack.pop();
                NFA[final_state].f = 1;
                NFAtoDFA(NFA, DFA, start_state);
                minizeDFA(DFA, MiniDFA);
                M_DFA.push_back(MiniDFA);
            }
            else
            {
                SplitString(lex, v, "/");
                LEX nlex;
                nlex.token = v[0];
                nlex.yyval = v.size() > 1 ? v[1] : "";
                LEXs.push_back(nlex);
            }
        }
        catch (string e)
        {
            cout << e << endl;
        }
    }
    inpLex.close();

    //读取待匹配字符串进行match
    ifstream inpTest("test.txt");
    string str;
    vector<LEX> out;
    while (inpTest >> str)
    {
        try
        {
            int i = 0;
            for (; i < M_DFA.size(); i++)
            {
                if (parsingStr(str, M_DFA[i].size() - 1, M_DFA[i]))
                {
                    LEX t = LEXs[i];
                    t.value = str;
                    t.yyval = t.yyval != "" ? t.yyval : str;
                    out.push_back(t);
                    break;
                }
            }
            if (i == M_DFA.size())
                throw "\"" + str + "\" is not matched.";
        }
        catch (string e)
        {
            cout << e << endl;
        }
    }
    //输出
    cout << "-------------------------------------------------------------------------" << endl;
    cout << "|\ttoken\t\t|\tyylval\t\t|\tvalue\t\t|" << endl;
    cout << "-------------------------------------------------------------------------" << endl;

    for (LEX l : out)
    {
        cout << "|\t" << l.token << "\t\t|\t" << l.yyval << "\t\t|\t" << l.value << "\t\t|" << endl;
    }
    cout << "-------------------------------------------------------------------------" << endl;

    return 0;
}
//函数定义
void SplitString(const string &s, vector<string> &v, const string &c)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
}
static int getPriority(char c)
{
    switch (c)
    {
    case '*':
        return 3;
    case '.':
        return 2;
    case '|':
        return 1;
    default:
        return 0;
    }
}
void getNormal(int i, vector<NFAstate> &NFA)
{
    int sizeNFA = NFA.size();
    NFA.push_back(NFA_init);
    NFA.push_back(NFA_init);
    NFA[sizeNFA].a[i].push_back(sizeNFA + 1);
    workStack.push(sizeNFA);
    sizeNFA++;
    workStack.push(sizeNFA);
    sizeNFA++;
}

void getSelect(vector<NFAstate> &NFA)
{
    int sizeNFA = NFA.size();
    NFA.push_back(NFA_init);
    NFA.push_back(NFA_init);
    int end1 = workStack.top();
    workStack.pop();
    int start1 = workStack.top();
    workStack.pop();
    int end2 = workStack.top();
    workStack.pop();
    int start2 = workStack.top();
    workStack.pop();
    NFA[sizeNFA].e.push_back(start1);
    NFA[sizeNFA].e.push_back(start2);
    NFA[end2].e.push_back(sizeNFA + 1);
    NFA[end1].e.push_back(sizeNFA + 1);
    workStack.push(sizeNFA);
    sizeNFA++;
    workStack.push(sizeNFA);
    sizeNFA++;
}

void getConnect(vector<NFAstate> &NFA)
{
    int end1 = workStack.top();
    workStack.pop();
    int start1 = workStack.top();
    workStack.pop();
    int end2 = workStack.top();
    workStack.pop();
    int start2 = workStack.top();
    workStack.pop();
    NFA[end2].e.push_back(start1);
    workStack.push(start2);
    workStack.push(end1);
}

void getRepeat(vector<NFAstate> &NFA)
{
    int sizeNFA = NFA.size();
    NFA.push_back(NFA_init);
    NFA.push_back(NFA_init);
    int end = workStack.top();
    workStack.pop();
    int start = workStack.top();
    workStack.pop();
    NFA[sizeNFA].e.push_back(start);
    NFA[sizeNFA].e.push_back(sizeNFA + 1);
    NFA[end].e.push_back(start);
    NFA[end].e.push_back(sizeNFA + 1);
    workStack.push(sizeNFA);
    sizeNFA++;
    workStack.push(sizeNFA);
    sizeNFA++;
}
void getClosure(int state, set<int> &workSet, vector<NFAstate> &NFA)
{
    for (unsigned int i = 0; i < NFA[state].e.size(); i++)
    {
        if (workSet.count(NFA[state].e[i]) == 0)
        {
            workSet.insert(NFA[state].e[i]);
            getClosure(NFA[state].e[i], workSet, NFA);
        }
    }
}
set<int> state_change(int i, set<int> &workSet, vector<NFAstate> &NFA)
{
    set<int> temp;
    for (std::set<int>::iterator it = workSet.begin(); it != workSet.end(); ++it)
    {
        for (unsigned int j = 0; j < NFA[*it].a[i].size(); j++)
        {
            temp.insert(NFA[*it].a[i][j]);
        }
    }
    return temp;
}
bool parsingStr(const string str, int state, vector<DFAstate> &MiniDFA)
{

    int strPos = 0;
    while (strPos < str.size())
    {
        //找到STR中str每一位的对应位置，此位置即为mini_dfa中每一个状态下转化的触发字符
        string::size_type pos = STR.find(str[strPos++]);
        state = MiniDFA[state].a[pos];
    }
    //若最终状态停留在终态则返回值为1，否则为0
    return MiniDFA[state].f;
}
string reToStandardRE(string re)
{
    string stand;
    for (int i = 0; i < re.size(); i++)
    {
        if (re[i] == '[')
        {
            while (re[i] != ']')
            {
                string temp = "(";
                string::size_type innerPos;
                if ((innerPos = re.find('-', i)) != string::npos)
                {
                    string::size_type startPos = STR.find(re[innerPos - 1]);
                    string::size_type endPos = STR.find(re[innerPos + 1]);
                    if (startPos == string::npos || endPos == string::npos)
                        throw "character not supported in []";
                    else
                        for (; startPos <= endPos; startPos++)
                        {
                            char c = STR[startPos];
                            temp.push_back(c);
                            temp.push_back('|');
                        }
                    temp.pop_back();
                    temp += ")";
                }
                stand += temp;
                i += 4;
            }
        }
        else if (re[i] == '.')
        {
            string temp = "(";
            for (int j = 0; j < STRLEN; j++)
            {
                char c = STR[j];
                temp.push_back(c);
                temp.push_back('|');
            }
            temp.pop_back();
            stand += temp + ")";
        }
        else if (re[i] == '+')
        {
            stand.push_back(stand[stand.size() - 1]);
            stand.push_back('*');
        }
        else
        {
            stand.push_back(re[i]);
        }
    }

    return stand;
}
string reToPostfix(string re)
{
    string regexp = "";
    char c, c2;
    for (unsigned int i = 0; i < re.size(); i++)
    {
        c = re[i];
        if (i + 1 < re.size())
        {
            c2 = re[i + 1];
            regexp += c;
            if (c != '(' && c2 != ')' && c != '|' && c2 != '|' && c2 != '*')
            {
                regexp += '.';
            }
        }
    }
    regexp += re[re.size() - 1];
    string postfix = "";
    stack<char> op;
    for (unsigned int i = 0; i < regexp.size(); i++)
    {
        string::size_type position = STR.find(regexp[i]);
        if (position != string::npos)
        {
            postfix += regexp[i];
        }
        else if (regexp[i] == '(')
        {
            op.push(regexp[i]);
        }
        else if (regexp[i] == ')')
        {
            while (op.top() != '(')
            {
                postfix += op.top();
                op.pop();
            }
            op.pop();
        }
        else
        {
            while (!op.empty())
            {
                c = op.top();
                if (getPriority(c) >= getPriority(regexp[i]))
                {
                    postfix += op.top();
                    op.pop();
                }
                else
                    break;
            }
            op.push(regexp[i]);
        }
    }
    while (!op.empty())
    {
        postfix += op.top();
        op.pop();
    }
    return postfix;
}
void postfixToNFA(string postfix, vector<NFAstate> &NFA)
{
    for (unsigned int i = 0; i < postfix.size(); i++)
    {

        string::size_type position = STR.find(postfix[i]);
        if (position != string::npos)
        {
            getNormal(position, NFA);
        }
        else if (postfix[i] == '*')
        {
            getRepeat(NFA);
        }
        else if (postfix[i] == '.')
        {
            getConnect(NFA);
        }
        else if (postfix[i] == '|')
        {
            getSelect(NFA);
        }
        else
        {
            throw postfix[i] + " is not supported.";
        }
    }
}
void NFAtoDFA(vector<NFAstate> &NFA, vector<DFAstate> &DFA, int start_state)
{
    set<int> workSet;
    queue<set<int>> workQueue;
    map<set<int>, int> workMap; //nfa状态集合与其标号对应关系
    workMap[workSet] = -1;
    set<int> temp;
    int currentState = 0;
    workSet.clear();
    workSet.insert(0);
    getClosure(start_state, workSet, NFA); //初始化I0
    if (workMap.count(workSet) == 0)
    {
        workMap[workSet] = currentState++;
        workQueue.push(workSet);
    }
    int p = 0; //指向当前dfa
    bool isTerminal = false;
    while (workQueue.size() != 0)
    {
        DFA.push_back(DFA_init);
        workSet.empty();
        workSet = workQueue.front();
        //判断当前nfa状态集是否为终态
        isTerminal = false;
        for (set<int>::iterator it = workSet.begin(); it != workSet.end(); ++it)
        {
            if (NFA[*it].f == true)
                isTerminal = true;
        }
        DFA[p].f = isTerminal;
        //开始状态转化
        for (int i = 0; i < STRLEN; i++)
        {
            workSet = workQueue.front();
            temp = state_change(i, workSet, NFA);
            workSet = temp;
            for (set<int>::iterator it = workSet.begin(); it != workSet.end(); ++it)
            {
                getClosure(*it, workSet, NFA);
            }
            if (workMap.count(workSet) == 0)
            {
                workMap[workSet] = currentState;
                workQueue.push(workSet);
                DFA[p].a[i] = currentState++;
            }
            else
            {
                DFA[p].a[i] = workMap.find(workSet)->second;
            }
            temp.clear();
        }
        workQueue.pop();
        p++;
    }
    for (int i = 0; i < p; i++)
    {
        for (int j = 0; j < STRLEN; j++)
            if (DFA[i].a[j] == -1)
                DFA[i].a[j] = p;
    }
    DFA.push_back(DFA_init);
    for (int j = 0; j < STRLEN; j++)
        DFA[p].a[j] = p;
}
void minizeDFA(vector<DFAstate> &DFA, vector<DFAstate> &MiniDFA)
{
    vector<int> group(DFA.size());                   // 状态数量
    vector<vector<int>> partition(2, vector<int>()); // 将dfa状态分为两部分

    // 初始化状态
    partition[0].push_back(0);
    for (int i = 1; i < (int)group.size(); i++)
    {
        if (DFA[i].f == false)
        {
            group[i] = 0;
            partition[0].push_back(i);
        }
        else
        {
            group[i] = 1;
            partition[1].push_back(i);
        }
    }
    //初始化后part无终态时清除part[1]
    if (!partition[1].size())
        partition.erase(partition.end());

    bool chk = true; //判断循环是否结束
    int strt = 0;    //开始状态
    while (chk)
    {
        chk = false;
        for (int i = 0; i < partition.size(); i++)
        {
            for (int j = 0; j < STRLEN; j++)
            {

                vector<pair<int, int>> trans(partition[i].size()); //trans[0].first表示状态推导后是否为终态
                for (int k = 0; k < partition[i].size(); k++)
                {
                    if (DFA[partition[i][k]].a[j] >= 0)
                        //对于每个dfa状态将其产生的状态是否为终态标记
                        trans[k] = make_pair(group[DFA[partition[i][k]].a[j]], partition[i][k]);
                    else
                        trans[k] = make_pair(-1, partition[i][k]);
                }
                sort(trans.begin(), trans.end());

                //分割每个部分
                if (trans[0].first != trans[trans.size() - 1].first)
                {
                    chk = true;
                    int k, m = partition.size() - 1;
                    partition[i].clear();
                    partition[i].push_back(trans[0].second);
                    //将具有相同推导后状态的状态组合在一起
                    for (k = 1; k < trans.size() && (trans[k].first == trans[k - 1].first); k++)
                    {
                        partition[i].push_back(trans[k].second);
                    }
                    while (k < trans.size())
                    {
                        if (trans[k].first != trans[k - 1].first)
                        {
                            partition.push_back(vector<int>());
                            m++;
                        }
                        group[trans[k].second] = m;
                        partition[m].push_back(trans[k].second);
                        k++;
                    }
                }
            }
        }
    }

    for (int i = 0; i < partition.size(); i++)
    {
        for (int j = 0; j < partition[i].size(); j++)
        {
            if (partition[i][j] == 0)
                strt = i;
        }
    }
    for (int i = 0; i < (int)partition.size(); i++)
    {
        MiniDFA.push_back(DFA_init);
        for (int j = 0; j < STRLEN; j++)
        {
            MiniDFA[MiniDFA.size() - 1].a[j] = (DFA[partition[i][0]].a[j] >= 0) ? group[DFA[partition[i][0]].a[j]] : -1;
            MiniDFA[MiniDFA.size() - 1].f = DFA[partition[i][0]].f;
        }
    }
}
