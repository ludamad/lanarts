#pragma  once
#include "oxygine-include.h"
#include <string>

namespace oxygine
{
    namespace text
    {
        class Node;

        class TextBuilder
        {
        public:
            TextBuilder();
            virtual ~TextBuilder();

            text::Node* parse(const std::string& str);
        };
    }
}