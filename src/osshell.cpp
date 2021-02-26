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

using namespace std;

struct stat;

//namespace fs = std::filesystem;

void allocateArrayOfCharArrays(char ***array_ptr, size_t array_length, size_t item_size);
void freeArrayOfCharArrays(char **array, size_t array_length);
void splitString(std::string text, char d, char **result);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    // `os_path_list` supports up to 16 directories in PATH, 
    //     each with a directory name length of up to 64 characters
    char **os_path_list;
    allocateArrayOfCharArrays(&os_path_list, 16, 64);
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);

    // Define ostream, string and vector used for history logging
    ofstream file;
    string input;
    vector<string> history;

    // Example code for how to loop over NULL terminated list of strings
    /*
    int i = 0;
    while (os_path_list[i] != NULL)
    {
        printf("PATH[%2d]: %s\n", i, os_path_list[i]);
        i++;
    }
    */

    // Attempt to read from a log file of the command history
    try{
        // input file stream is from history_log.txt
        ifstream readHistory("history_log.txt");
        string curLine;
        while(getline(readHistory, curLine)){
            // while there is still a next line, read that line into the history vector
            history.push_back(curLine);
        }
    } catch (exception e) {
        // If the file doesn't exist (exception thrown), there is no log to be read from
    };

    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    // Allocate space for input command lists
    // `command_list` supports up to 32 command line parameters, 
    //     each with a parameter string length of up to 128 characters
    char **command_list;
    allocateArrayOfCharArrays(&command_list, 32, 128);
    
    // Repeat:
    while(1) {
        // Print prompt for user input: "osshell> " (no newline)
        printf("osshell> ");
        // Get user input for next command
        getline(cin,input);

        if(input == "exit") {
            // append the most recent command to the history vector
            history.push_back(input);
            // create a log file to save history, iterator for writing to it
            ofstream file;
            file.open("history_log.txt");
            ostream_iterator<string> iterator(file, "\n");
            // write to the log file, then exit the program
            copy(history.begin(), history.end(),iterator);
            file.close();
            break;

        } else if(input == "history") {
            // print out all values for history
            for(int i = 0; i < history.size(); i++) {
                cout << i << ": " << history[i] << endl; 
            }
            // add "history" to the list of previous commands
            history.push_back(input);

        } else if(input == "" || input.find_first_not_of(' ') == string::npos) {
            // If the command entered is blank (no characters) or just a newline, don't do anything in this iteration
            continue;

        } else if(1 == 0/** If command is history N, print previous N commands **/) {
            // ...

        } else {
            cout << "Entered all other commands scope" << endl;
            // Save the command, whatever it was   
            history.push_back(input);

            // For all other commands, check if an executable by that name is in one of the PATH directories

            // If yes, execute it

            // If no, print error statement: "<command_name>: Error command not found" (do include newline)
        }
    }
    // Free allocated memory
    freeArrayOfCharArrays(os_path_list, 16);
    //causes a seg fault
    //freeArrayOfCharArrays(command_list, 32);

    return 0;
}

/*
   array_ptr: pointer to list of strings to be allocated
   array_length: number of strings to allocate space for in the list
   item_size: length of each string to allocate space for
*/
void allocateArrayOfCharArrays(char ***array_ptr, size_t array_length, size_t item_size)
{
    int i;
    *array_ptr = new char*[array_length];
    for (i = 0; i < array_length; i++)
    {
        (*array_ptr)[i] = new char[item_size];
    }
}

/*
   array: list of strings to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        delete[] array[i];
    }
    delete[] array;
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: NULL terminated list of strings (char **) - result will be stored here
*/
void splitString(std::string text, char d, char **result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::vector<std::string> list;
    std::string token;
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
                    list.push_back(token);
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
                    list.push_back(token);
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
        list.push_back(token);
    }

    for (i = 0; i < list.size(); i++)
    {
        strcpy(result[i], list[i].c_str());
    }
    result[list.size()] = NULL;
}
