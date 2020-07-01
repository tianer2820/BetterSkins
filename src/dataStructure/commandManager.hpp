#if !defined(COMMAND_MANAGER_H)
#define COMMAND_MANAGER_H

#include <list>
#include "command.hpp"

class CommandManager
{
public:
    static CommandManager *create();
    static void destruct();
    static CommandManager *getInstance();

    void add(Command *command);

    bool undo();
    bool redo();

protected:
    static int instance_count;
    static CommandManager *instance_ptr;
    static const int largest_num = 32;

    list<Command *> command_list;
    list<Command *>::iterator next_command;

    /**
     * return true if the command is valid, false if the command is not.
     * this will auto delete all command after the invalid command.
     */
    bool checkValidCommand();

    CommandManager();
    ~CommandManager();
};


#endif // COMMAND_MANAGER_H
