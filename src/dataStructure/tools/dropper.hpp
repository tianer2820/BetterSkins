#if !defined(DROPPER_H)
#define DROPPER_H

#include "Tool.hpp"

class Dropper : public Tool
{
public:
    Dropper()
    {
        setProperty("SIZE", 1);
        tool_type = ToolType::DROPPER;
    }

    virtual void moveTo(int x, int y) {}
    virtual void penDown() {}
    virtual void penUp() {}
    virtual void setFunctionalKeys(bool shift, bool ctrl) {}

protected:
};

#endif // DROPPER_H
