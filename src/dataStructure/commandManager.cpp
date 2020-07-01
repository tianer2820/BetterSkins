
#include <list>
#include "command.hpp"
#include "commandManager.hpp"

CommandManager *CommandManager::create()
{
    if (instance_count > 0)
    {
        return instance_ptr;
    }
    instance_ptr = new CommandManager();
    instance_count += 1;
    return instance_ptr;
}
void CommandManager::destruct()
{
    if (instance_count > 0)
    {
        delete instance_ptr;
        instance_count = 0;
    }
}
CommandManager *CommandManager::getInstance()
{
    if (instance_count == 0)
    {
        // CommandManager::create();
        throw "Command manager is not created";
    }
    return instance_ptr;
}

void CommandManager::add(Command *command)
{
    // empty the later command
    while (next_command != command_list.end())
    {
        delete *next_command;
        next_command = command_list.erase(next_command);
    }
    // add the new command
    command_list.push_back(command);
    next_command = command_list.end();

    if (command_list.size() > largest_num)
    {
        delete *(command_list.begin()); // delete the command object
        command_list.pop_front();       // limit the command list size
    }
}

bool CommandManager::undo()
{
    if (next_command == command_list.begin())
    {
        // no command to undo
        return false;
    }
    next_command--;
    while (checkValidCommand() == false)
    {
        if (next_command == command_list.begin())
        {
            // no command to undo
            return false;
        }
        next_command--;
    }
    (*next_command)->undo();
    return true;
}
bool CommandManager::redo()
{
    if (next_command == command_list.end())
    {
        //no command to redo
        return false;
    }
    if (checkValidCommand() == false)
    {
        return false;
    }
    (*next_command)->redo();
    next_command++;
    return true;
}

list<Command *> command_list;
list<Command *>::iterator next_command;

/**
     * return true if the command is valid, false if the command is not.
     * this will auto delete all command after the invalid command.
     */
bool CommandManager::checkValidCommand()
{
    if ((*next_command)->isValid())
    {
        return true;
    }
    while (next_command != command_list.end())
    {
        delete *next_command;
        next_command = command_list.erase(next_command);
    }
    return false;
}

CommandManager::CommandManager()
{
    next_command = command_list.end();
}
CommandManager::~CommandManager()
{
    for (auto i = command_list.begin(); i != command_list.end(); i++)
    {
        delete *i;
    }
}

int CommandManager::instance_count = 0;
CommandManager *CommandManager::instance_ptr = nullptr;
