#include "node.h"

// class Node

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
        // data += std::to_string(index) + " [label=\"" + tag + ":" + value + "\"];\n";
        data += std::to_string(index) + " [label=\"" + tag + "\"];\n";
        for (auto i = children.begin(); i != children.end(); i++)
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