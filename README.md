# System Monitoring Tool - Concurrency & Signals
### This is the CSCB09 Assignment 3: System Monitoring Tool. 
It is a command-line program that monitors system information such as CPU and memory usage, and displays it to the user. It also provides the ability to sort and filter this information based on various parameters.

### Table of Contents
System Requirements
Installation
Usage
Contribution
License
### System Requirements
This program is designed to run on a Linux operating system, and requires the following dependencies to be installed:

gcc compiler
make
ncurses library
### Installation
Clone the repository to your local machine.
shell
Copy code
$ git clone https://github.com/your-username/B09-Assignment-3-System-Monitoring-Tool-Concurrency-Signals.git
Change directory to the cloned repository.
shell
Copy code
$ cd B09-Assignment-3-System-Monitoring-Tool-Concurrency-Signals/
Compile the program using the Makefile.
go
Copy code
$ make
Run the program.
shell
Copy code
$ ./monitor
### Usage
The program is a command-line tool that can be run with the following options:

less
Copy code
Usage: ./monitor [-h|--help] [-p|--processes] [-t|--threads] [-m|--memory] [-c|--cpu] [-s|--sort] [-f|--filter <value>] [-r|--refresh <value>]
-h, --help: Displays the help menu and exits.
-p, --processes: Displays the list of all processes currently running on the system.
-t, --threads: Displays the number of threads for each process currently running on the system.
-m, --memory: Displays the memory usage for each process currently running on the system.
-c, --cpu: Displays the CPU usage for each process currently running on the system.
-s, --sort: Sorts the list of processes based on the given parameter (memory, cpu, pid, name).
-f, --filter: Filters the list of processes to only display those that contain the given value in their name.
-r, --refresh: Sets the refresh interval in seconds for updating the information displayed on the screen.
### Contribution
This project is open to contributions from anyone. If you would like to contribute to the project, please follow these steps:

Fork the repository.
Clone the repository to your local machine.
Create a new branch for your changes.
Make your changes and commit them to your branch.
Push your changes to your forked repository.
Create a pull request to merge your changes into the original repository.
### License
This project is licensed under the MIT License - see the LICENSE file for details.
