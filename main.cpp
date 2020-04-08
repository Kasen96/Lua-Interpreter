#include "main.h"
#include "node.h"

#include <fstream>

extern Node root;

void yy::parser::error(const string &err)
{
    cout << "Error: " << err << endl;
}

int main(int argc, char* argv[])
{
    // read file from cmd
    if (argc > 1)
    {
        FILE *file = std::fopen(argv[1], "r");
        if (file)
            yyin = file;
        else
            yyin = stdin;
        yy::parser parser;
        parser.parse();
        std::fclose(file);
    }

    // generate parse.dot
    std::ofstream outfile;
    outfile.open("parse.dot", std::ios::out);
    string data = "digraph {\n";
    root.drawDigraph(1, data); // dot index starts by 1
    data += "}";
    outfile << data << endl;
    outfile.close();

    root.run();
    return 0;
}