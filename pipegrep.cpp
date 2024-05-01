/*
 * Authors: Meghan Grayson and Vaishnavi Karaguppi
 * Date: April 29, 2024
 * 
 * This program uses threads and condition variables to implement a pipelined grep application. The pipelined grep
 * will search all files in the current directory for a given string. The user will also be able to filter files out
 * of the search based on file size, file user id, and file group id. The application is constructed as a five stage
 * pipeline where there is one thread per stage. Threads between consecutive stages use the producer-consumer paradigm
 * to communicate and synchronize their actions.
 * 
 * The program is organized into the following architecture:
 * Stage1 -> buff1 -> Stage2 -> buff2 -> Stage3 -> buff3 -> Stage4 -> buff4 -> Stage5
 * 
 * Usage: ./pipegrep <buffsize> <filesize> <uid> <gid> <string>
 * If <filesize>, <uid>, or <gid> are -1, that field is ignored
 * 
 * */

#include <iostream>
#include <fstream>
#include <thread>
#include <string.h>
#include <cstdio>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "buffer.h"


using namespace std;


/* Declaring global variables */
int buffsize, filesize, uid, gid; // Info for the filter function
string searchStr; // String to be grep'd
thread workers[5]; // One thread per stage
string doneToken = "done"; // Token to let each stage know they're finished

/* The four buffers */
// See buffer.h and buffer.cpp for implementation details
producerConsumer::buffer *buff1;
producerConsumer::buffer *buff2;
producerConsumer::buffer *buff3;
producerConsumer::buffer *buff4;

/* 
 * This function is Stage 1 of the grep search. 
 * The worker thread in this stage recurses through the current directory and adds filenames to the first buffer (buff1).
 * 
 * Pre-condition: Buff1 is empty.
 * Post-condition: Buff1 contains all current directory filenames plus a "done" token to be passed along the stages.
 * */
void acquireFilenames() {
    DIR *thisDirectory; // A pointer to the current directory
    struct dirent *entry; // A pointer to the current directory entry
    struct stat entry_info;
    if((thisDirectory = opendir("./")) != NULL) { // Try to open the current directory
        while((entry = readdir(thisDirectory)) != NULL) { // Read all entries until none left
            lstat(entry->d_name, &entry_info); // To get information about the file type
            /* Only want to add regular files to the buffer */
            if((entry_info.st_mode & S_IFMT) == S_IFREG) {  // If it's a regular file, add to the buffer
                buff1->add(entry->d_name); // Add to buffer
            }
        }
        //closedir(thisDirectory); // Close the directory
        buff1->add(doneToken); // Done adding files so add the done token
    }
    else {
        cerr << "Unable to open the current directory." << endl;
        exit(EXIT_FAILURE);
    }
    return;
}


/* 
 * This function is Stage 2 of the grep search. 
 * In this stage, the thread will read filenames from buff1 and filter out files according to the values provided on the 
 * command-line for ⟨filesize⟩, ⟨uid⟩, and ⟨gid⟩ as described above. Those files not filtered out are added to buff2.
 * 
 * Pre-condition: Buff1 contains all current directory filenames
 * Post-condition: Buff2 contains all filenames to be searched based on the provided arguments, and Buff1 is empty.
 * */
void fileFilter() {
    /* Temporary holders for processing */
    string filename;
    struct stat file_info;
    /* Determine which items need to be checked */
    bool filterSize = (filesize != -1);
    bool filterUID = (uid != -1);
    bool filterGID = (gid != -1);
    while((filename = buff1->remove()) != doneToken) { // Read the next filename until done
        lstat(filename.c_str(), &file_info); // Get info about the current file
        if(filterSize) {
            if(file_info.st_size <= filesize) continue; // Skip files less than or equal to given filesize
        }
        if(filterUID) {
            if(file_info.st_uid != uid) continue; // Skip files not owned by the given uid
        }
        if(filterGID) {
            if(file_info.st_gid != gid) continue; // Skip files not owned by the given gid
        }
        buff2->add(filename); // Add files that make it to this point
    }
    buff2->add(doneToken); // Finished, add done token
    return;
}


/* 
 * This function is a helper for Stage 3 of the grep search. 
 * It checks if the file indicated by filename is a binary file (i.e. an executable or .o file)
 * so that it may be skipped by the line generation stage.
 * 
 * param: filename -- The string that contains the path from the current directory to the file.
 * return: true if the file is a binary file. false if it is not a binary file.
 * */
bool isBinaryFile(string filename) {
    int charCount = 0; // Number of bytes read
    int nonAsciiCount = 0; // Number of non-ascii bytes read
    char c; // Holder for next byte
    ifstream file; 
    file.open(filename.c_str()); // Open the file specified by filename
    if(!file.is_open()) { // Clean exit if unable to open file
        cerr << "Unable to open file" << endl;
        exit(EXIT_FAILURE);
    }

    while(charCount < 300 && file.get(c)) { // Read until 100 bytes or no more to read
        if(!isascii(c)) nonAsciiCount++; // Increment non-ascii char count
        charCount++; // increment count
    }
    file.close(); // Close the file
    float ratio = (float)nonAsciiCount / (float)charCount;
    if(ratio > 0.01) return true; // If more than 1% non-ascii characters were read, return true
    return false; // Otherwise return true
}

/* 
 * This function is Stage 3 of the grep search. 
 * The thread in this stage reads each filename from buff2 and adds the lines in this file to buff3.
 * 
 * Pre-condition: Buff2 contains all filenames to be searched based on the provided arguments.
 * Post-condition: Buff3 contains lines from each of the filtered files, and Buff2 is empty.
 * */
void lineGeneration() {
    ifstream file; // for holding the next file after opening
    string filename; // for holding the name of the current file
    string line; // for holding the current line

    while((filename = buff2->remove())!= doneToken) { // Read the next file until done
        if(isBinaryFile(filename)) continue; // Check if it's a binary file -- if so, skip
        file.open(filename.c_str()); // Open the next file
        if(!file.is_open()) { // Clean exit if unable to open the file
            cerr << "Unable to open file" << endl;
            exit(EXIT_FAILURE);
        }
        while(getline(file, line)) { // Read the file line by line until EOF
            buff3->add(line); // Add it to the buffer
        }
        file.close(); // Remember to close and clear to reuse the ifstream
        file.clear();
    }
    buff3->add(doneToken); // Finished, add done token
    return;
}


/* 
 * This function is Stage 4 of the grep search. 
 * In this stage, the thread reads the lines from buff3 and determines if any given one contains ⟨string⟩
 * in it. If it does, it adds the line to buff4.
 * 
 * Pre-condition: Buff3 contains lines from each of the filtered files.
 * Post-condition: Buff4 contains lines that contain (string), and Buff3 is empty.
 * */
void lineFilter() {
    string line; // for holding the entire current line
    while((line = buff3->remove()) != doneToken) { // Get the next line until finished
        if(line.find(searchStr) != string::npos) { // If the string is found in the current line
            buff4->add(line); // Add it to the next buffer
        }
    }
    buff4->add(doneToken); // Finished, add done token
    return;
}


/* 
 * This function is Stage 5 of the grep search. 
 * In this stage, the thread simply removes lines from buff4 and prints them to stdout.
 * 
 * Pre-condition: Buff4 contains lines that contain (string)
 * Post-condition: Buff4 is empty.
 * */
void output() {
    string line; // for holding the line to be printed
    int totalFound = 0; // For holding the number of matches
    while((line = buff4->remove()) != doneToken) {
        cout << line << endl; // Print the line
        totalFound++; // Increment total found
    }
    cout << "***** You found " << totalFound << " matches *****" << endl;
    return;
}


/* The main function */
int main(int argc, char** argv) {
    if (argc < 6) {
        cerr << "Too few arguments provided. Usage: ./pipegrep <buffsize> <filesize> <uid> <gid> <string>" << endl;
        exit(EXIT_FAILURE);
    }

    // Read the arguments provided by the user and assign
    buffsize = stoi(argv[1]);
    filesize = stoi(argv[2]);
    uid = stoi(argv[3]);
    gid = stoi(argv[4]);
    searchStr = argv[5];


    // An exception will be thrown if the buffsize, filesize, uid, or gid are not integers

    // Verify the arguments before proceeding
    if (buffsize <= 0 || filesize < -1 || uid < -1 || gid < -1 || searchStr.empty()) {
        cerr << "Invalid arguments provided. Usage: ./pipegrep <buffsize> <filesize> <uid> <gid> <string>" << endl;
        exit(EXIT_FAILURE);
    }

    /* Initialize the buffers */
    buff1 = new producerConsumer::buffer(buffsize);
    buff2 = new producerConsumer::buffer(buffsize);
    buff3 = new producerConsumer::buffer(buffsize);
    buff4 = new producerConsumer::buffer(buffsize);

    /* Create the worker threads */
    workers[0] = thread(acquireFilenames); // Stage 1 worker
    workers[1] = thread(fileFilter); // Stage 2 worker
    workers[2] = thread(lineGeneration); // Stage 3 worker
    workers[3] = thread(lineFilter); // Stage 4 worker
    workers[4] = thread(output); // Stage 5 worker

    /* Wait for all threads to terminate */
    for(int i = 0; i < 5 ; i++) {
        workers[i].join();
    }

    /* Delete all buffers */
    buff1 = NULL;
    buff2 = NULL;
    buff3 = NULL;
    buff4 = NULL;

    return EXIT_SUCCESS;
}