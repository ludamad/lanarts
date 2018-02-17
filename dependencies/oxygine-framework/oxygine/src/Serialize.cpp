#include "Serialize.h"
#include "Sprite.h"
#include "TextField.h"
#include "ColorRectSprite.h"
#include "ClipRectActor.h"
#include "ProgressBar.h"
#include "SlidingActor.h"
#include "Polygon.h"
#include "MaskedSprite.h"
#include "Box9Sprite.h"
#include "WebImage.h"

namespace oxygine
{
    //#define S(s) #s
    //#define SS(s) ##s
#define CREATE(T) if (!strcmp(#T, type)) return new T

    spActor creator::create(const char* type) const
    {
        CREATE(Sprite);
        CREATE(TextField);
        CREATE(Actor);
        CREATE(ProgressBar);
        CREATE(Box9Sprite);
        CREATE(SlidingActor);
        CREATE(ColorRectSprite);
        CREATE(ClipRectActor);
        CREATE(Polygon);
        CREATE(MaskedSprite);
        CREATE(WebImage);


        return 0;
    }

    static void link(pugi::xml_node node, spActor actor, spActor root)
    {
        deserializeLinkData data;
        data.node = node;
        data.root = root;
        actor->deserializeLink(&data);

        spActor child = actor->getFirstChild();
        node = node.first_child();
        while (child)
        {
            link(node, child, root);
            node = node.next_sibling();
            child = child->getNextSibling();
        }
    }

    void deserializeLinkData::link(pugi::xml_node node, spActor actor)
    {
        oxygine::link(node, actor, actor);
    }

    spActor deserializedata::deser(pugi::xml_node node, const creator* factory)
    {
        deserializedata d;
        d.node = node;
        d.factory = factory;
        const char* name = node.name();
        spActor actor = factory->create(name);
        actor->deserialize(&d);

        return actor;
    }
}
