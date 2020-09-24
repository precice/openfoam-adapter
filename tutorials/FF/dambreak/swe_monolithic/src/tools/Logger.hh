/**
 * @file
 * This file is part of SWE.
 *
 * @author Alexander Breuer (breuera AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Dipl.-Math._Alexander_Breuer)
 * @author Sebastian Rettenberger (rettenbs AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Sebastian_Rettenberger,_M.Sc.)
 *
 * @section LICENSE
 *
 * SWE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 * Collection of basic logging routines.
 */

#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#if (defined USEMPI && !defined CUDA)
#include <mpi.h>
#endif

#include <map>
#include <string>
#include <iostream>
#include <ctime>

namespace tools {
  class Logger;
}

class tools::Logger {
//private:

  /**
   * Stream, which prints the time in the beginning.
   *
   * @return extended std::cout stream.
   */
  std::ostream& timeCout() {
    //get current time
    time_t rawTime;
    time(&rawTime);

    //convert time to a human readable format
    std::string humanReadableTime = ctime(&rawTime);

    //remove new-line character
    humanReadableTime.erase(humanReadableTime.end() - 1);

    //return the stream
    return std::cout << humanReadableTime;
  }

  //! definition of the process rank (0 == master process)
  int processRank;

  //! definition of the program name
  const std::string programName;

  //! definition of the welcome message
  const std::string welcomeMessage;

  //! definition of the copyrights
  const std::string copyRights;

  //! definition of the finish message
  const std::string finishMessage;

  //! definition of a mid delimiter
  const std::string midDelimiter;

  //! definition of a large delimiter
  const std::string largeDelimiter;

  //! definition of indentation
  const std::string indentation;

  //! Clocks
  std::map<std::string, clock_t> clocks;

  //! Timer
  std::map<std::string, double> timer;

  //! wall clock time: cpu, communication, IO
  double wallClockTime;

  /**
   * Print the number of 1D quantities.
   *
   * @param i_nX size in x-direction.
   * @param i_quantity definition of the qantity.
   */
  void printNumber1d( const int i_nX,
                      const std::string i_quantity ) {
    std::cout << "Number of " << i_quantity << ": " << i_nX << std::endl;
  }

  /**
   * Print the number of 2D quantities.
   *
   * @param i_nX size in x-direction.
   * @param i_nY size in y-direction.
   * @param i_quantity definition of the qantity.
   */
  void printNumber2d( const int i_nX,
                      const int i_nY,
                      const std::string i_quantity ) {
    std::cout << "Number of " << i_quantity << ": " << i_nX << " * " << i_nY
              << " = " << i_nX*i_nY << std::endl;
  }

  public:
    /**
     * The Constructor.
     * Prints the welcome message (process rank 0 only).
     *
     * @param i_processRank rank of the constructing process.
     * @param i_programName definition of the program name.
     * @param i_welcomeMessage definition of the welcome message.
     * @param i_startMessage definition of the start message.
     * @param i_simulationTimeMessage definition of the simulation time message.
     * @param i_executionTimeMessage definition of the execution time message.
     * @param i_cpuTimeMessage definition of the CPU time message.
     * @param i_finishMessage definition of the finish message.
     * @param i_midDelimiter definition of the mid-size delimiter.
     * @param i_largeDelimiter definition of the large delimiter.
     * @param i_indentation definition of the indentation (used in all messages, except welcome, start and finish).
     */
    Logger( const int i_processRank = 0,
            const std::string i_programName = "SWE",
            const std::string i_welcomeMessage = "Welcome to",
            const std::string i_copyRights =  "\n\nSWE Copyright (C) 2012-2013\n"
                                              "  Technische Universitaet Muenchen\n"
                                              "  Department of Informatics\n"
                                              "  Chair of Scientific Computing\n"
                                              "  http://www5.in.tum.de/SWE\n"
                                              "\n"
                                              "SWE comes with ABSOLUTELY NO WARRANTY.\n"
                                              "SWE is free software, and you are welcome to redistribute it\n"
                                              "under certain conditions.\n"
                                              "Details can be found in the file \'gpl.txt\'.",
            const std::string i_finishMessage = "finished successfully.",
            const std::string i_midDelimiter = "\n------------------------------------------------------------------\n",
            const std::string i_largeDelimiter = "\n*************************************************************\n",
            const std::string i_indentation = "\t" ):
            processRank(i_processRank),
            programName(i_programName),
            welcomeMessage(i_welcomeMessage),
            copyRights(i_copyRights),
            finishMessage(i_finishMessage),
            midDelimiter(i_midDelimiter),
            largeDelimiter(i_largeDelimiter),
            indentation(i_indentation) {

#ifndef USEMPI
      // Since we have one static logger, we do not know the MPI rank in this
      // constructor. When using MPI, the process rank has to be set first,
      // before printing the welcome message.
  	  printWelcomeMessage();
#endif
    }

    /**
     * The Destructor.
     * Prints the finish message (process rank 0 only).
     */
    virtual ~Logger() {
      #ifndef USEMPI
        printFinishMessage();
      #endif
      std::cout.flush();
    }

    /**
     * Print the welcome message.
     */
    void printWelcomeMessage() {
      if(processRank == 0) {
        std::cout << largeDelimiter
                  << welcomeMessage << " "
                  << programName
                  << copyRights
                  << largeDelimiter;
      }
    }

    /**
     * Print the finish message.
     */
    void printFinishMessage() {
      if(processRank == 0) {
        std::cout << largeDelimiter
                  << programName << " "
                  << finishMessage
                  << largeDelimiter;
      }
    }

    /**
     * Default output stream of the logger.
     *
     * @return extended (time + indentation) std::cout stream.
     */
    std::ostream& cout() {
      return timeCout() << indentation
        #ifdef USEMPI
        <<  "process " << processRank << " - "
        #endif
        ;
    }

    /**
     * Set the process rank.
     *
     * @param i_processRank process rank.
     */
    void setProcessRank( const int i_processRank ) {
      processRank = i_processRank;
    }


    /**
     * Print an arbitrary string.
     *
     * @param i_string some string.
     */
    void printString(const std::string i_string) {
      if (processRank == 0 )
      timeCout() << indentation
                << i_string << std::endl;
    }

    /**
     * Print the number of processes.
     * (process rank 0 only)
     *
     * @param i_numberOfProcesses number of processes.
     * @param i_processesName name of the processes.
     */
    void printNumberOfProcesses( const int i_numberOfProcesses,
                                 const std::string i_processesName="MPI processes" ) {
      if (processRank == 0 )
      timeCout() << indentation
                << "Number of " << i_processesName << ": "
                << i_numberOfProcesses << std::endl;
    }

    /**
     * Print the number of cells.
     * (process rank 0 only)
     *
     * @param i_nX number of cells in x-direction.
     * @param i_nY number of cells in y-direction.
     * @param i_cellMessage cell message.
     */
    void printNumberOfCells( const int i_nX,
                             const int i_nY,
                             const std::string i_cellMessage="cells") {
      if(processRank == 0) {
        timeCout() << indentation;
        printNumber2d(i_nX, i_nY, i_cellMessage);
      }
    }

    /**
     * Print the number of cells per Process.
     *
     * @param i_nX number of cells in x-direction.
     * @param i_nY number of cells in y-direction.
     */
    void printNumberOfCellsPerProcess( const int i_nX, const int i_nY ) {
      timeCout() << indentation << "process " << processRank << " - ";
      printNumber2d(i_nX, i_nY, "cells");
    }

    /**
     * Print the size of a cell
     *
     * @param i_dX size in x-direction.
     * @param i_dY size in y-direction.
     * @param i_unit measurement unit.
     */
    void printCellSize( const float i_dX, const float i_dY, const std::string i_unit="m" ) {
      if(processRank == 0) {
        timeCout() << indentation
                   <<"Cell size: " << i_dX << i_unit <<" * " << i_dY << i_unit
                   << " = " << i_dX*i_dY << " " << i_unit << "^2" << std::endl;
      }
    }


    /**
     * Print the number of defined blocks.
     * (process rank 0 only)
     *
     * @param i_nX number of blocks in x-direction.
     * @param i_nY number of blocks in y-direction.
     */
    void printNumberOfBlocks( const int i_nX, const int i_nY ) {
      if(processRank == 0) {
        timeCout() << indentation;
        printNumber2d(i_nX, i_nY, "blocks");
      }
    }

    /**
     * Print the start message.
     * (process rank 0 only)
     */
    void printStartMessage( const std::string i_startMessage = "Everything is set up, starting the simulation." ) {
      if(processRank == 0) {
        std::cout << midDelimiter;
        timeCout() << indentation << i_startMessage;
        std::cout << midDelimiter;
      }
    }

    /**
     *  Print current simulation time.
     *  (process rank 0 only)
     *
     * @param i_time time in seconds.
     */
    void printSimulationTime( const float i_time,
                              const std::string i_simulationTimeMessage = "Simulation at time" ) {
      if(processRank == 0) {
        timeCout() << indentation
                  << i_simulationTimeMessage << ": " << i_time << " seconds." << std::endl;
      }
    }

    /**
     * Print the creation of an output file.
     *
     * @param i_fileName name of the file.
     * @param i_blockX block position in x-direction.
     * @param i_blockY block position in y-direction.
     * @param i_fileType type of the output file.
     */
    void printOutputFileCreation( const std::string i_fileName,
                                  const int i_blockX , const int i_blockY,
                                  const std::string i_fileType = "netCDF" ) {
      timeCout() << indentation << "process " << processRank << " - "
                << "creating " << i_fileType << " file " << i_fileName << " for block " << i_blockX << ", " << i_blockY << "." << std::endl;
    }

    /**
     * Print the current output time.
     *
     * @param i_time time in seconds.
     * @param i_outputTimeMessage output message.
     */
    void printOutputTime( const float i_time,
                          const std::string i_outputTimeMessage="Writing output file at time" ) {
      if(processRank == 0) {
        timeCout() << indentation
                  << i_outputTimeMessage << ": " << i_time << " seconds" << std::endl;
      }
    }

    /**
     * Print the statics message.
     *
     * @param i_statisticsMessage statistics message.
     */
    void printStatisticsMessage( const std::string i_statisticsMessage="Simulation finished. Printing statistics for each process." ) {
      if(processRank == 0) {
        std::cout << midDelimiter;
        timeCout() << indentation << i_statisticsMessage;
        std::cout << midDelimiter;
      }
    }

    /**
     * Print solver statistics
     *
     * @param i_firstSolverCounter times the first solver was used.
     * @param i_secondSolverCounter times the second solver was used.
     * @param i_blockX position of the block in x-direction
     * @param i_blockY position of the block in y-direction
     * @param i_firstSolverName name of the first solver.
     * @param i_secondSolverName name of the second solver.
     */
    void printSolverStatistics( const long i_firstSolverCounter,
                                const long i_secondSolverCounter,
                                const int i_blockX=0,
                                const int i_blockY=0,
                                const std::string i_firstSolverName="f-Wave solver",
                                const std::string i_secondSolverName="Augemented Riemann solver" ) {
      timeCout() << indentation << "process " << processRank << " - "
                 << "Solver Statistics for block " << i_blockX << ", " << i_blockY << ":"<< std::endl;
      timeCout() << indentation << "process " << processRank << " - " << indentation
                 << "Times the " << i_firstSolverName << " was used: " << i_firstSolverCounter << std::endl;
      timeCout() << indentation << "process " << processRank << " - " << indentation
                 << "Times the " << i_secondSolverName << " was used: " << i_secondSolverCounter << std::endl;
      timeCout() << indentation << "process " << processRank << " - " << indentation
                 << "In Total: " << i_firstSolverCounter + i_secondSolverCounter << std::endl;
    }

    /**
     * Update a timer
     *
     * @param i_name Name of timer
     */
    void updateTime(const std::string &i_name) {
    	timer[i_name] += (clock() - clocks.at(i_name))/(double)CLOCKS_PER_SEC;
    }

    /**
     * Reset a clock to the current time
     *
     * @param i_name Name of timer/clock
     */
    void resetClockToCurrentTime(const std::string &i_name) {
    	clocks[i_name] = clock();
    }

    /**
     * Initialize the wall clock time.
     *
     * @param i_wallClockTime value the wall block time will be set to.
     */
    void initWallClockTime( const double i_wallClockTime ) {
      wallClockTime = i_wallClockTime;
    }

    /**
     * Print the elapsed wall clock time.
     *
     * @param i_wallClockTime wall clock time message.
     */
    void printWallClockTime( const double i_wallClockTime,
                             const std::string i_wallClockTimeMessage = "wall clock time" ) {
      timeCout() << indentation << "process " << processRank << " - "
                 << i_wallClockTimeMessage << ": "
                 << i_wallClockTime - wallClockTime
                 << " seconds"<< std::endl;
    }

    /**
     * Print elapsed time.
     *
     * @param i_name Name of the timer
     * @param i_message time message.
     */
    void printTime(const std::string &i_name, const std::string &i_message ) {
      timeCout() << indentation << "process " << processRank << " - "
                << i_message << ": "
                << timer.at(i_name) << " seconds"<< std::endl;
    }

    /**
     * Get elapsed time
     *
     * @param i_name Name of the time
     * @return elapsed time
     */
    double getTime(const std::string &i_name) {
    	return timer.at(i_name);
    }

    /**
     * Print number of iterations done
     *
     * @param i_iterations Number of iterations done
     * @param i_interationMessage Iterations done message
     */
    void printIterationsDone(unsigned int i_iterations, std::string i_iterationMessage = "iterations done")
    {
    	if (processRank == 0) {
    		timeCout() << indentation << i_iterations
    			<< ' ' << i_iterationMessage << std::endl;
    	}
    }

    /**
     * Print number of element updates done
     *
     * @param i_iterations Number of iterations done
     * @param i_interationMessage Iterations done message
     */
    void printElementUpdatesDone(unsigned int i_iterations, const int i_nX, const int i_nY, 
	                             const std::string &i_name, const std::string i_iterationMessage = "element updates per second done")
    {
    	if (processRank == 0) {
    		timeCout() << indentation << double(i_iterations)*i_nX*i_nY / timer.at(i_name)
    			<< ' ' << i_iterationMessage << std::endl;
    	}
    }

  public:
    /** The logger all classes should use */
    static Logger logger;
};


#endif /* LOGGER_HPP_ */
