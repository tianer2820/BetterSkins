#if !defined(COMMAND_MANAGER_H)
#define COMMAND_MANAGER_H

#include <list>
#include "command.hpp"

class CommandManager{
    public:
    static CommandManager* create(){
        if(instance_count > 0){
            return instance_ptr;
        }
        instance_ptr = new CommandManager();
        instance_count += 1;
    }
    static void destruct(){
        if(instance_count > 0){
            delete instance_ptr;
            instance_count = 0;
        }
    }
    static CommandManager* getInstance(){
        if(instance_count == 0){
            // CommandManager::create();
            throw "Command manager is not created";
        }
        return instance_ptr;
    }

    void add(Command* command){
        // empty the later command
        while (next_command != command_list.end())
        {
            delete *next_command;
            next_command = command_list.erase(next_command);
        }
        // add the new command
        command_list.push_back(command);
        next_command = command_list.end();

        if(command_list.size() > largest_num){
            delete *(command_list.begin()); // delete the command object
            command_list.pop_front(); // limit the command list size
        }
    }

    bool undo(){
        if(next_command == command_list.begin()){
            // no command to undo
            return false;
        }
        next_command --;
        (*next_command)->undo();
        return true;
    }
    bool redo(){
        if(next_command == command_list.end()){
            //no command to redo
            return false;
        }
        (*next_command)->redo();
        next_command++;
        return true;
    }

    protected:
    static int instance_count;
    static CommandManager* instance_ptr;
    static const int largest_num = 32;

    list<Command*> command_list;
    list<Command*>::iterator next_command;

    CommandManager(){
        next_command = command_list.end();
    }
    ~CommandManager(){
        for(auto i = command_list.begin(); i != command_list.end(); i++){
            delete *i;
        }
    }
};

int CommandManager::instance_count = 0;
CommandManager* CommandManager::instance_ptr = nullptr;

#endif // COMMAND_MANAGER_H