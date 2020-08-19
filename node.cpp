#include "node.h"

// class Node

// umap (variable, number)
// unordered_map for fast search
static std::unordered_map<string, Node> umap;

Node::Node() // Bison needs this
{
    tag = "uninitialised";
    value = "uninitialised";
}

Node::Node(string t, string v) : tag(t), value(v) {}

/*
void Node::dump(int depth = 0)
{
    for (int i = 0; i < depth; i++)
    {
        cout << "  ";
    }
    cout << tag << ":" << value << endl;
    for (auto i = children.begin(); i != children.end(); i++)
    {
        (*i).dump(depth + 1);
    }
}
*/

void Node::drawDigraph(int index, string &data)
{
    static int count = index; // count the number of nodes
    // not leaf node
    if (!children.empty())
    {
        // index [label="tag"];
        if (value != "")
            data += std::to_string(index) + " [label=\"" + tag + ":" + value + "\"];\n";
        else
            data += std::to_string(index) + " [label=\"" + tag + "\"];\n";
        for (auto i = children.begin(); i != children.end(); ++i)
        {
            // node points to its children
            // index -> count;
            data += std::to_string(index) + " -> " + std::to_string(++count) + ";\n";
            (*i).drawDigraph(count, data); // traverse children
        }
    }
    else // leaf node, tag -> value for clarity
    {
        /*
        index [label="tag"];
        tagindex [label="value"];
        index ->tagindex;
        */
        data += std::to_string(index) + " [label=\"" + tag + "\"];\n";
        data += tag + std::to_string(index) + " [label=\"" + value + "\"];\n";
        data += std::to_string(index) + " -> " + tag + std::to_string(index) + ";\n";
    }
}

/**
 * the parser part.
 * a really bad design,
 * not every part needs to return a node,
 * multi-level nesting of if - else,
 * some parts are time-consuming,
 * function is incomplete,
 * ...
*/
Node Node::run()
{
    if (tag == "chunk" || tag == "explist" || tag == "varlist" || tag == "block")
    {
        Node result;
        for(auto i = children.begin(); i != children.end(); ++i)
        {
            result = (*i).run();
        }
        return result;
    }
    else if (tag == "functioncall")
    {
        Node var = getChildNode(0); 
        if (var.getChildNode(0).value == "print") // NAME.value == "print"
        {
            print(getChildNode(1)); // print(explist);
            return Node("print", "end");
        }
        else if (var.getChildNode(0).value == "io" && var.getChildNode(1).value == "read") // io.read()
        {
            Node explist_child = getChildNode(1).getChildNode(0); // getChildNode(1) == explist; explist -> STRING
            if (explist_child.value == "*number" || explist_child.value == "*n") // io.read("*number"||"*n") read a number
            {
                return io_read();
            }
            // io.read("*a"), io.read("*l") ...
        }
        else if (var.getChildNode(0).value == "io" && var.getChildNode(1).value == "write") // io.write()
        {
            for (auto &explist_child : getChildNode(1).children)
            {
                io_write(explist_child);
            }
            return Node("io.write", "end");
        }
        else // call function
        {
            string func_name = getChildNode(0).run().value;
            Node func = readMap(func_name);
            Node exp = getChildNode(1).getChildNode(0).run();
            return callFunc(func, exp).run();
        }
    }
    else if (tag == "stat")
    {
        if (value == "FOR") // for loop
        {
            // omit getChildNode(1).tag == "ASSIGN"
            store2Map(getChildNode(0).value, getChildNode(2)); // store i = ?;
            double i = getArgsNum(getChildNode(0));
            double n; 
            if (getChildNode(3).tag == "exp")
            {
                n = std::stod(getChildNode(3).run().value);
            }
            else
            {
                n = getArgsNum(getChildNode(3).run());
            }
            do
            {
                getChildNode(4).run();
                ++i;
                store2Map(getChildNode(0).value, Node("num_exp", std::to_string(i))); // update i
            } while ( n + 1 - i > 0 );
            return Node("stat:FOR", "end");
        }
        else if (value == "IF")
        {
            // first node is exp
            string temp = getChildNode(0).run().value;
            if (temp == "true")
            {
                return getChildNode(1).run();
            }
            else
            {
                if (children.size() == 3)
                {
                    return getChildNode(2).run();
                }
                else
                {
                    return Node("stat:IF", "end");
                }
            }          
        }
        else if (value == "REPEAT")
        {
            do 
            {
                getChildNode(0).run();
            } while (getChildNode(1).run().value == "false");
            return Node("stat:REPEAT", "end");
        }
        else if (value == "FUNCTION")
        {
            string func_name = getChildNode(0).getChildNode(0).value;
            store2Map(func_name, getChildNode(1)); // store funcbody
            return Node("stat:FUNCTION", "end");
        }
        else // common assign stat
        {
            Node sec_node = getChildNode(1);
            if (sec_node.tag == "ASSIGN")
            {
                assign(getChildNode(0), getChildNode(2));
                return Node("stat", "end");
            }
        }
    }
    else if (tag == "var")
    {
        if (value == "BRACKET")
        {
            Node list = readMap(getChildNode(0).run().value);
            int index = std::stoi(std::to_string(getArgsNum(getChildNode(1).run())));
            return list.getChildNode(index - 1); // lua for i stats by 1
        }
        else
        {
            return getChildNode(0); // return NAME   
        }
    }
    else if (tag == "exp")
    {
        if (value == "true" || value == "false")
        {
            return *this;
        }
        else if (getChildNode(0).tag == "unop") // unary operator
        {
            // #
            if (getChildNode(0).value == "#")
            {
                Node list = readMap(getChildNode(1).run().value);
                return Node("exp#", std::to_string(list.children.size()));
            }
        }
        else if (getChildNode(1).tag == "binop") // binary operator
        {
            Node sec_node = getChildNode(1);
            if (sec_node.value == "==")
            {
                if (getChildNode(2).value == "true" || getChildNode(2).value == "false") // var == bool
                {
                    string result = readMap(getChildNode(0).run().value).value == getChildNode(2).value ? "true" : "false";
                    return Node("exp==", result);
                }
                else // num == num
                {
                    double left = getArgsNum(getChildNode(0).run());
                    double right = getArgsNum(getChildNode(2).run());
                    string result = (std::fabs(left - right) < 0.000001) ? "true" : "false"; // ==, floating point comparison
                    return Node("exp==", result);
                }
            }
            else if (sec_node.value == ">")
            {
                double left = getArgsNum(getChildNode(0).run());
                double right = getArgsNum(getChildNode(2).run());
                string result = left - right > 0.000001 ? "true" : "false";
                return Node("exp>", result);
            }
            else if (sec_node.value == "<")
            {
                double left = getArgsNum(getChildNode(0).run());
                double right = getArgsNum(getChildNode(2).run());
                string result = right - left > 0.000001 ? "true" : "false";
                return Node("exp<", result);
            }
            else // + - * / ^ %
            {
                double left = getArgsNum(getChildNode(0).run());
                double right = getArgsNum(getChildNode(2).run());
                double result;

                if (sec_node.value == "+")
                    result = left + right;
                if (sec_node.value == "-")
                    result = left - right;
                if (sec_node.value == "*")
                    result = left * right;
                if (sec_node.value == "/")
                    result = left / right;
                if (sec_node.value == "^")
                    result = std::pow(left, right);
                if (sec_node.value == "%")
                    result = std::fmod(left, right);

                return Node("num_exp", std::to_string(result));
            }
        }
    }
    else if (tag == "num_exp")
    {
        return *this;
    }
    else if (tag == "tableconstructor")
    {
        return getChildNode(0); // return fieldlist
    }
    else if (tag == "laststat")
    {
        return getChildNode(0).run();
    }
}

Node Node::getChildNode(int i)
{
    // using vector to reduce time complexity
    std::vector<Node> nodes { std::begin(children), std::end(children) }; // list initialization in c++11
    return nodes[i];
}

void Node::print(Node node)
{
    if (node.tag == "STRING")
    {
        cout << node.value << endl;
    }
    else
    {
        for(auto n : node.children)
        {
            cout << std::fixed << std::setprecision(1) << getArgsNum(n.run()) << " ";
        }
        cout << endl;
    }
}

double Node::getArgsNum(Node node)
{
    string result = "";
    if (node.tag == "num_exp")
    {
        result = node.value;
    }
    else if (node.tag == "NAME")
    {
        auto iterator = umap.find(node.value); // node.value is the variable name, e.g. NAME -> x, node.value is x
        result = (iterator -> second).value; // value is the specific number the variable has. e.g. {x, Node("num_exp", "1")} The value is 1.
    }
    return std::stod(result);
}

void Node::assign(Node varlist, Node explist)
{
    int size = varlist.children.size();
    if (size == 1)
    {
        store2Map(varlist.run().value, explist.run()); // (x | y | z, Node("xxx", "number"))
    }
    else // swap list node, the code below wastes too much time?
    {
        Node explist_result = Node();
        for (auto &n : explist.children)
        {
            explist_result.children.push_back(n.run());
        }
        for (int i = 0; i < size; i++)
        {
            Node left = varlist.getChildNode(i);
            Node right = explist_result.getChildNode(i);
            int index = std::stoi(std::to_string(getArgsNum(left.getChildNode(1).run())));
            store2Table(left.getChildNode(0).run().value, index - 1, right);
        }  
    }  
}

void Node::store2Map(string key, Node node)
{
    auto iterator = umap.find(key);
    if (iterator == umap.end())
    {
        umap.insert({key, node});
    }
    else // if key exists, update value
    {
        iterator->second = node;
    }
}

// bad design, time-consuming
void Node::store2Table(string table_name, int index, Node value)
{
    Node table = readMap(table_name);
    Node new_table = Node("fieldlist", "");
    for (int i = 0; i < table.children.size(); i++)
    {
        if (index == i)
        {
            new_table.children.push_back(value);
        }
        else
        {
            new_table.children.push_back(table.getChildNode(i));
        }       
    }
    store2Map(table_name, new_table);
}

Node Node::readMap(string key)
{
    return umap.find(key)->second;
}

Node Node::io_read()
{
    string value;
    getline(std::cin, value);
    try
    {
        string val = std::to_string(std::stoi(value)); // e.g. "3.14" -> "3"
        return Node("io_read", val);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        exit(-1);
    }
}

Node Node::io_write(Node node)
{
    if (node.tag == "STRING")
    {
        cout << node.value;
    }
    else
    {
        cout << std::fixed << std::setprecision(1) << getArgsNum(node.run());
    }
    return Node("io_write", "end");
}

Node Node::callFunc(Node funcbody, Node exp)
{
    string para_name = funcbody.getChildNode(0).getChildNode(0).value;
    Node func_block = funcbody.getChildNode(1).getChildNode(0);
    return replace(func_block, para_name, exp);
}

// replace the formal parameter with actual parameter
// bad design, time-consuming
Node Node::replace(Node target, string name, Node exp)
{
    if (target.children.size() > 0)
    {
        std::list<Node> temp;
        for (auto &i : target.children)
        {
            temp.push_back(replace(i, name, exp));
        }
        target.children = temp;
        return target;
    }
    else if (target.value == name)
    {
        return exp;
    }
    else
    {
        return target;
    }
}
