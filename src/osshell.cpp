#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sys/wait.h>
#include <sys/types.h>

using namespace std;

struct stat;

namespace fs = std::filesystem;

void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    char* os_path = getenv("PATH");
    // Vector of strings to hold the list of paths obtained by getenv()
    vector<string> os_path_list;
    // Split the string holding the OS paths, delimited by ':', and store in os_path_list
    splitString(os_path, ':', os_path_list);
    // File output stream for writing to the history log file 
    ofstream file;
    // Holds the user's raw input as a string
    string input;
    // Used by history to record the user's command as it was entered (spaces, tabs, etc.)
    string cmd;
    // Stores UNDIVIDED string representations of raw commands entered by the user
    vector<string> history;
    // To store command user types in after it is split into its various parameters
    vector<string> command_list; 
    // Holds the char* representation of the input arguments
    char** arguments;
    // Allocates memory to arguments
    arguments = (char **)malloc(50 *sizeof(char *));

    /***********************************/
    /** READING FROM HISTORY LOG FILE **/
    /***********************************/
    try{
        // cmd file stream is from history_log.txt
        ifstream readHistory("history_log.txt");
        string curLine;
        while(getline(readHistory, curLine)){
            // While there is still a next line, read that line into the history vector
            history.push_back(curLine);
        }
    } catch (exception e) {
        // If the file doesn't exist (exception thrown), there is no log to be read from
    };

    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    /********************************/
    /** BEGIN SHELL OPERATION LOOP **/
    /********************************/
    while(1) {
        printf("osshell> ");
        // Get user input, store in 'input'
        getline(cin,input);
        // Store the raw input in cmd for recording in history (unmodified)
        cmd = input;
        // Split the input, delimited by a space, and store in command_list
        splitString(input, ' ', command_list); // <- something is going on here, segmentation fault on second iteration
    
        // Same as previous comment, but stored in arguments
        int i = 0;
        char *token = strtok(const_cast<char*>(input.c_str()), " ");
        while (token != NULL)
        {
            arguments[i] = token;
            token = strtok(NULL, " ");
            i++;
        }

        //There was an issue with the code using arguments from previous inputs
        //But magically this seems to have solved it *fingers crossed*
        arguments[i] = NULL;
        
        //remove quotations
        for(int x = 0; x < i; x++){
            for(int j = 0; j < strlen(arguments[x]); j++){
                if(arguments[x][j] == '"'){
                    //likely won't work with two quotes next to each other
                    for(int k = j; k < strlen(arguments[x]); k++){
                        arguments[x][k] = arguments[x][k+1];
                    }
                }
            }
        }

        /*******************/
        /** EMPTY COMMAND **/
        /*******************/
        if(command_list.size() == 0) {
            // If the command entered is blank (no characters) or just a newline, don't do anything in this iteration
            continue;

        /**********/
        /** EXIT **/
        /**********/
        } else if(command_list[0] == "exit") {
            // Append the most recent command to the history vector
            history.push_back(cmd);
            // Create a log file to save history, 
            ofstream file;
            // Open the log file
            file.open("history_log.txt");
            // Create an iterator for writing to the log file
            ostream_iterator<string> iterator(file, "\n");
            // Write to the log file, then exit the program
            copy(history.begin(), history.end(), iterator);
            file.close();
            break;

        /*************/
        /** HISTORY **/
        /*************/
        } else if(command_list[0] == "history") {
            if(command_list.size() == 1) {
                // Display the entire history
                if(history.size() == 0) {
                    cout << "There are no previous commands to display." << endl;
                } else {                
                    for(int i = 0; i < history.size(); i++) {
                        cout << "  " << i + 1 << ": " << history[i] << endl; 
                    }
                }
                // Add the history command to the history
                history.push_back(cmd);

            } else if(command_list[1].find_first_not_of("0123456789") == string::npos) {
                // Parse a numeric argument for history if a digit is detected in the second string of command_list
                int historyNumericArg = stoi(command_list[1]);
                // Add the command to the list of previous commands
                history.push_back(cmd);

                if(history.size() == 0) {
                    cout << "There are no previous commands to display." << endl;
                } else {
                    if(historyNumericArg > history.size()) {
                        cout << "Only " << history.size() << " command(s) in the history log. Displaying all entries." << endl;
                    }
                    // Print the number of previous commands specified
                    for(int i = 0; i < historyNumericArg; i++) {
                        if(i > history.size() - 1) { break; }
                        cout << "  " << history.size() - historyNumericArg + i << ": " << history[history.size() - historyNumericArg + i - 1] << endl; 
                    }
                }                

            } else if(command_list[1] == "clear") {
                // Clear the history vector completely if the user entered the 'clear' command after history
                if(history.size() == 0) {
                    cout << "History is already empty." << endl;
                }
                history.clear();

            } else {
                // If history's arguments are invalid
                history.push_back(cmd);
                cout << "Error: history expects an integer > 0 (or 'clear')" << endl;
            }

        }

        /***************************************/
        /** COMMANDS STARTING WITH "." OR "/" **/
        /***************************************/
        else if (command_list[0][0] == '/' || command_list[0][0] == '.') {
            // If the command starts with "." or "/", look for that path
            fs::path fp = arguments[0];
            if(fs::exists(fp)){
                if(((fs::status(fp).permissions() & fs::perms::owner_exec) != fs::perms::none) && !fs::is_directory(fp)){
                    //Fork
                    pid_t parent = getpid();
                    pid_t pid = fork();

                    if(pid == -1){
                        //fork error
                        std::cout << "Fork Error\n";
                    } else if(pid > 0) {
                        //parent waits for child
                        int status;
                        waitpid(pid, &status, 0);
                    } else if (pid == 0) {
                        execv(const_cast<char*>(fp.c_str()), arguments);
                    }
                } else {//path not executable
                    std::cout << cmd << ": Error command not found\n";
                }
            } else {//path not found
                std::cout << cmd << ": Error command not found\n";
            }

        } else {
            // For all other commands, check if an executable by that name is in one of the PATH directories
            string exe;
            string bin = "/bin/";
            fs::path p = bin + arguments[0];
            int i = 0;

            for(int i = 0; i < os_path_list.size(); i++) {
                
                for(const auto& part : fs::directory_iterator(os_path_list[i]))
                {
                    if(part.path().compare(p) == 0)
                    {
                        exe = part.path();
                        break;
                    }
                }
                
            }

            // If yes, execute it
            if(exe.compare("") != 0)
            {
                //Fork
                pid_t parent = getpid();
                pid_t pid = fork();

                if(pid == -1){
                    //fork error
                    std::cout << "Fork Error\n";
                } else if(pid > 0) {
                    int status;
                    waitpid(pid, &status, 0);
                } else if (pid == 0){
                    //Exec
                    execv(const_cast<char*>(exe.c_str()), arguments);
                }
            }
            // If no, print error statement: "<command_name>: Error command not found" (do include newline)
            else {
                std::cout << cmd << ": Error command not found\n";
            }
            // Save the command, whatever it was   
            history.push_back(cmd);
            
        }
    }
    return 0;
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{   
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;
    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}
