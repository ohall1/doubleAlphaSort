#include <iostream>
#include <fstream>
#include <sstream>
#include <list>


//ROOT Libraries
#include "TFile.h"

#include "AnalysisProcess.cpp"

void Usage(char *progname){
    std::cout << "Usage: doubleAlpha -c configFile -o OutputFile" << std::endl;
    exit;
}
int main(int argc, char **argv) {
    std::cout << "Hello, World!" << std::endl;

    std::string runName, line;
    std::string configFile, alphaParameters;
    std::string inFile, outFile, userOutFile;
    std::string dataDir, outputDir;
    int runNum, subRunStart, subRunEnd;
    runNum=0;
    subRunStart = 0;
    subRunEnd = 0;

    std::list<std::string> alphaFileList;

    if (argc > 3){
        std::cout << "Run time options:"<< std::endl;

        for (int i = 1; i <argc;i++){
            std::cout << argv[i] << std::endl;

            if ( (argv[i][0] == '-') || (argv[i][0] == '/') ) {
                switch( argv[i][1]){

                    case 'c':
                        configFile = argv[++i];
                        std::cout << "Configuration file: "<< configFile << std::endl;
                        break;

                    case 'o':
                        userOutFile = argv[++i];
                        std::cout << "User output file: " << userOutFile << std::endl;
                        break;

                    default:
                        Usage(argv[0]);
                        return -1;
                        break;

                }
            }
            else{
                Usage(argv[0]);
                return -1;
            }
        }
    }// End of reading in command line arguments
    else{
        Usage(argv[0]);
        return -1;
    }

    std::ifstream confFile(configFile.data());
    while ( confFile.good() ){
        getline(confFile,line);
        auto commentLine=line.find("#");
        std::string dummyVar;
        auto newLine=line.substr(0,commentLine);
        if(newLine.empty()){std::istringstream iss(line,std::istringstream::in);

            iss >> dummyVar;

            if (dummyVar == "alphaFile"){
                iss>>dummyVar;
                if ( alphaFileList.empty()){
                    alphaFileList.push_back(dummyVar);
                    std::cout << "Added file: " << dummyVar << " to be sorted" <<std::endl;
                }
                else {
                    std::cout << "alphaFile configure error" << std::endl;
                    std::cout << "alpha files already defined. Please only use one method of defining files" << std::endl;
                }
            }//End of AIDAFIle
            else if (dummyVar == "alphaList"){
                iss>>runName;
                iss>>runNum;
                iss>>subRunStart;
                iss>>subRunEnd;

                if (alphaFileList.empty()){
                    std::cout << "Adding files to be sorted:" << std::endl;
                    for(int i=subRunStart; i<=subRunEnd; i++){
                        dummyVar = runName + std::to_string(runNum) + "_" + std::to_string(i);
                        std::cout << dummyVar << std::endl;
                        alphaFileList.push_back(dummyVar);
                    }
                }
                else {
                    std::cout << "alphaList configure error" << std::endl;
                    std::cout << "alpha files already defined. Please only use one method of defining files" << std::endl;
                }
            }//End of AIDAList
            else if (dummyVar == "alphaConfig"){
                iss >> alphaParameters;
            }//End of AIDAConfig
        }//End of if (newLine)
    }// End of reading in configuration file
    confFile.close();

    AnalysisProcess analysisProcess;
    if(!analysisProcess.ReadParameters(alphaParameters)){
        std::cout << "Problem reading parameters file" << std::endl;
        return -1;
    }



    return 0;
}
