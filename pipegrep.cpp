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
#include <thread>
#include <string>

#include "buffer.h"


using namespace std;


/* Declaring global variables */
int buffsize, filesize, uid, gid;
string searchStr; // String to be grep'd
thread workers[5]; // One thread per stage

/* The four buffers */
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

}


/* 
 * This function is Stage 3 of the grep search. 
 * The thread in this stage reads each filename from buff2 and adds the lines in this file to buff3.
 * 
 * Pre-condition: Buff2 contains all filenames to be searched based on the provided arguments.
 * Post-condition: Buff3 contains lines from each of the filtered files, and Buff2 is empty.
 * */
void lineGeneration() {

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

}


/* 
 * This function is Stage 5 of the grep search. 
 * In this stage, the thread simply removes lines from buff4 and prints them to stdout.
 * 
 * Pre-condition: Buff4 contains lines that contain (string)
 * Post-condition: Buff4 is empty.
 * */
void output() {

}


/* The main function */
int main(int argc, char** argv) {
    if (argc < 6) {
        cerr << "Too few arguments provided. Usage: ./pipegrep <buffsize> <filesize> <uid> <gid> <string>" << endl;
        exit(1);
    }

    // Read the arguments provided by the user and assign
    buffsize = stoi(argv[1]);
    filesize = stoi(argv[2]);
    uid = stoi(argv[3]);
    gid = stoi(argv[4]);
    searchStr = argv[5];

    // An exception will be thrown if the buffsize, filesize, uid, or gid are not integers

    return 0;
}