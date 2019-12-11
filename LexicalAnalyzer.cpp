#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <queue>
#include <algorithm>
#include <fstream>
using namespace std;

string STR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const int STRLEN = 62;

struct nst
{
    vector<int> a[STRLEN], e;
    bool f = 0;
};

vector<nst> nfa;

struct dst
{
    int a[STRLEN] = {-1};
    bool f = 0;
};

vector<dst> dfa;
vector<dst> mini_dfa;
stack<int> st;

int nfa_size, dfa_size;
string dispregex;

struct nst init_nfa_state;
struct dst init_dfa_state;

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

int priority(char c)
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
                if (priority(c) >= priority(regexp[i]))
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
void character(int i)
{
    nfa.push_back(init_nfa_state);
    nfa.push_back(init_nfa_state);
    nfa[nfa_size].a[i].push_back(nfa_size + 1);
    st.push(nfa_size);
    nfa_size++;
    st.push(nfa_size);
    nfa_size++;
}

void union_()
{
    nfa.push_back(init_nfa_state);
    nfa.push_back(init_nfa_state);
    int d = st.top();
    st.pop();
    int c = st.top();
    st.pop();
    int b = st.top();
    st.pop();
    int a = st.top();
    st.pop();
    nfa[nfa_size].e.push_back(a);
    nfa[nfa_size].e.push_back(c);
    nfa[b].e.push_back(nfa_size + 1);
    nfa[d].e.push_back(nfa_size + 1);
    st.push(nfa_size);
    nfa_size++;
    st.push(nfa_size);
    nfa_size++;
}

void concatenation()
{
    int d = st.top();
    st.pop();
    int c = st.top();
    st.pop();
    int b = st.top();
    st.pop();
    int a = st.top();
    st.pop();
    nfa[b].e.push_back(c);
    st.push(a);
    st.push(d);
}

void kleene_star()
{
    nfa.push_back(init_nfa_state);
    nfa.push_back(init_nfa_state);
    int b = st.top();
    st.pop();
    int a = st.top();
    st.pop();
    nfa[nfa_size].e.push_back(a);
    nfa[nfa_size].e.push_back(nfa_size + 1);
    nfa[b].e.push_back(a);
    nfa[b].e.push_back(nfa_size + 1);
    st.push(nfa_size);
    nfa_size++;
    st.push(nfa_size);
    nfa_size++;
}

void postfix_to_nfa(string postfix)
{
    for (unsigned int i = 0; i < postfix.size(); i++)
    {

        string::size_type position = STR.find(postfix[i]);
        if (position != string::npos)
        {
            character(position);
        }
        else if (postfix[i] == '*')
        {
            kleene_star();
        }
        else if (postfix[i] == '.')
        {
            concatenation();
        }
        else if (postfix[i] == '|')
        {
            union_();
        }
        else
        {
            throw postfix[i] + "is not support";
        }
    }
}
void epsilon_closure(int state, set<int> &si)
{
    for (unsigned int i = 0; i < nfa[state].e.size(); i++)
    {
        if (si.count(nfa[state].e[i]) == 0)
        {
            si.insert(nfa[state].e[i]);
            epsilon_closure(nfa[state].e[i], si);
        }
    }
}
set<int> state_change(int i, set<int> &si)
{
    set<int> temp;
    for (std::set<int>::iterator it = si.begin(); it != si.end(); ++it)
    {
        for (unsigned int j = 0; j < nfa[*it].a[i].size(); j++)
        {
            temp.insert(nfa[*it].a[i][j]);
        }
    }
    return temp;
}

void nfa_to_dfa(set<int> &si, queue<set<int>> &que, int start_state)
{
    map<set<int>, int> mp; //nfa状态集合与其标号对应关系
    mp[si] = -1;
    set<int> temp;
    int ct = 0;
    si.clear();
    si.insert(0);
    epsilon_closure(start_state, si); //初始化I0
    if (mp.count(si) == 0)
    {
        mp[si] = ct++;
        que.push(si);
    }
    int p = 0; //指向当前dfa
    bool f1 = false;
    while (que.size() != 0)
    {
        dfa.push_back(init_dfa_state);
        si.empty();
        si = que.front();
        //判断当前nfa状态集是否为终态
        f1 = false;
        for (set<int>::iterator it = si.begin(); it != si.end(); ++it)
        {
            if (nfa[*it].f == true)
                f1 = true;
        }
        dfa[p].f = f1;
        //开始状态转化
        for (int i = 0; i < STRLEN; i++)
        {
            si = que.front();
            temp = state_change(i, si);
            si = temp;
            for (set<int>::iterator it = si.begin(); it != si.end(); ++it)
            {
                epsilon_closure(*it, si);
            }
            if (mp.count(si) == 0)
            {
                mp[si] = ct;
                que.push(si);
                dfa[p].a[i] = ct++;
            }
            else
            {
                dfa[p].a[i] = mp.find(si)->second;
            }
            temp.clear();
        }
        que.pop();
        p++;
    }
    for (int i = 0; i < p; i++)
    {
        for (int j = 0; j < STRLEN; j++)
            if (dfa[i].a[j] == -1)
                dfa[i].a[j] = p;
    }
    dfa.push_back(init_dfa_state);
    for (int j = 0; j < STRLEN; j++)
        dfa[p].a[j] = p;
}

void minimize_dfa()
{
    vector<int> grp(dfa.size());                // 状态数量
    vector<vector<int>> part(2, vector<int>()); // 将dfa状态分为两部分

    // 初始化状态
    part[0].push_back(0);
    for (int i = 1; i < (int)grp.size(); i++)
    {
        if (dfa[i].f == false)
        {
            grp[i] = 0;
            part[0].push_back(i);
        }
        else
        {
            grp[i] = 1;
            part[1].push_back(i);
        }
    }
    //初始化后part无终态时清除part[1]
    if (!part[1].size())
        part.erase(part.end());

    bool chk = true; //判断循环是否结束
    int strt = 0;    //开始状态
    while (chk)
    {
        chk = false;
        for (int i = 0; i < part.size(); i++)
        {
            for (int j = 0; j < STRLEN; j++)
            {

                vector<pair<int, int>> trans(part[i].size()); //trans[0].first表示状态推导后是否为终态
                for (int k = 0; k < part[i].size(); k++)
                {
                    if (dfa[part[i][k]].a[j] >= 0)
                        //对于每个dfa状态将其产生的状态是否为终态标记
                        trans[k] = make_pair(grp[dfa[part[i][k]].a[j]], part[i][k]);
                    else
                        trans[k] = make_pair(-1, part[i][k]);
                }
                sort(trans.begin(), trans.end());

                //分割每个部分
                if (trans[0].first != trans[trans.size() - 1].first)
                {
                    chk = true;
                    int k, m = part.size() - 1;
                    part[i].clear();
                    part[i].push_back(trans[0].second);
                    //将具有相同推导后状态的状态组合在一起
                    for (k = 1; k < trans.size() && (trans[k].first == trans[k - 1].first); k++)
                    {
                        part[i].push_back(trans[k].second);
                    }
                    while (k < trans.size())
                    {
                        if (trans[k].first != trans[k - 1].first)
                        {
                            part.push_back(vector<int>());
                            m++;
                        }
                        grp[trans[k].second] = m;
                        part[m].push_back(trans[k].second);
                        k++;
                    }
                }
            }
        }
    }

    for (int i = 0; i < part.size(); i++)
    {
        for (int j = 0; j < part[i].size(); j++)
        {
            if (part[i][j] == 0)
                strt = i;
        }
    }
    for (int i = 0; i < (int)part.size(); i++)
    {
        mini_dfa.push_back(init_dfa_state);
        for (int j = 0; j < STRLEN; j++)
        {
            mini_dfa[mini_dfa.size() - 1].a[j] = (dfa[part[i][0]].a[j] >= 0) ? grp[dfa[part[i][0]].a[j]] : -1;
            mini_dfa[mini_dfa.size() - 1].f = dfa[part[i][0]].f;
        }
    }
}

//解析每一个字符串，返回是否符合当前DFA词法
bool parsingStr(const string str, int state)
{

    int strPos = 0;
    while (strPos < str.size())
    {
        //找到STR中str每一位的对应位置，此位置即为mini_dfa中每一个状态下转化的触发字符
        string::size_type pos = STR.find(str[strPos++]);
        state = mini_dfa[state].a[pos];
    }
    //若最终状态停留在终态则返回值为1，否则为0
    return mini_dfa[state].f;
}

int main()
{
    //生成最小状态DNF集合
    ifstream inpLex("resources.txt");
    string lex;
    vector<vector<dst>> M_DFA;
    vector<string[3]> LEX;
    while (inpLex >> lex)
    {
        try
        {
            
            string re;
            cin >> re;
            string stand = reToStandardRE(re);
            cout << stand << endl;
            string postfix = reToPostfix(stand);
            cout << postfix << endl;
            postfix_to_nfa(postfix);
            int final_state = st.top();
            st.pop();
            int start_state = st.top();
            st.pop();
            nfa[final_state].f = 1;
            set<int> si;
            queue<set<int>> que;
            nfa_to_dfa(si, que, start_state);
            minimize_dfa();
            M_DFA.push_back(mini_dfa);
            mini_dfa.clear();
            // int start = mini_dfa.size() - 1;
            // bool is_match = parsingStr("9yyyyyyy", start);
            // cout << is_match << endl;
        }
        catch (const char *e)
        {
            cout << e << endl;
        }
    }
    //读取待匹配字符串进行match
    inpLex.close();
    ifstream inpTest("test.txt");
    string str;
    while (inpTest>>str)
    {
        for(vector<dst> a:M_DFA){
            if(parsingStr(str,a.size()-1)){

            }
        }
        
    }
    

}
