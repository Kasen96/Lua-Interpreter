#include "node.h"

// class Node

// umap (variable, number)
// unordered_map for fast search
static std::unordered_map<string, string> umap;

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

// bad design, not all parts need to return a node
Node Node::run()
{
    if (tag == "chunk" || tag == "explist" || tag == "varlist")
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
    }
    else if (tag == "stat")
    {
        Node sec_node = getChildNode(1);
        if (sec_node.tag == "ASSIGN")
        {
            assign(getChildNode(0), getChildNode(2));
            return Node("stat", "end");
        }
    }
    else if (tag == "var")
    {
        return getChildNode(0); // return NAME
    }
    else if (tag == "exp")
    {
        // + - * / ^ %
        Node sec_node = getChildNode(1);
        if (sec_node.tag == "binop")
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
    else if (tag == "num_exp")
    {
        return *this;
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
    for(auto n : node.children)
    {
        cout << std::fixed << std::setprecision(1) << getArgsNum(n.run()) << " ";
    }
    cout << endl;
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
        auto iterator = umap.find(node.value);
        result = iterator -> second;
    }
    return std::stod(result);
}

void Node::assign(Node varlist, Node explist)
{
    if (varlist.children.size() == 1)
    {
        store2Map(varlist.run().value, explist.run().value); // (x | y | z, number)
    }
}

void Node::store2Map(string key, string value)
{
    auto iterator = umap.find(key);
    if (iterator == umap.end())
    {
        umap.insert({key, value});
    }
    else // if key exists, update value
    {
        iterator->second = value;
    }
}
