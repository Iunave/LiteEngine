#include "Definitions.hpp"
#include "Object/Object.hpp"

OBJECT_CLASS(OWow) : public OObject
{
    OBJECT_BODY(OObject)
};

int main()
{
    OObject* Object{new OWow};
    OWow* Wow{ObjectCast<OWow*>(Object)};
    DoNotOptimize(Wow);
}
