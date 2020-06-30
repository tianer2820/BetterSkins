#if !defined(SELECTION_TOOL_H)
#define SELECTION_TOOL_H

#include "Tool.hpp"
#include "../layer.hpp"
#include "../../color/color.h"

/**
 * simple pen
 * R, G, B, A, SIZE
 */
class SelectionTool : public Tool
{
public:
    SelectionTool()
    {
        tool_type = ToolType::SELECT;
    }
    virtual void moveTo(int x, int y)
    {
        
    }
    virtual void penDown()
    {

    }
    virtual void penUp()
    {

    }
    virtual void setFunctionalKeys(bool shift, bool ctrl)
    {
    }

protected:

};

#endif // SELECTION_TOOL_H
