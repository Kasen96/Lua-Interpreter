#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <list>
#include <string>

using std::string;
using std::cout;
using std::endl;

class Node
{
    public:
        string tag, value;
        std::list<Node> children;

        Node();
        Node(string t, string v);

        //void dump(int);
        void drawDigraph(int, string &);
};

#endif