//
// Created by bacox on 15/05/2020.
//
#include <iostream>
#include <Orchestrator.h>
#include <filesystem>
#include <cxxopts.h>


/**
 * Run ./ndg --help for help
 */
int main(int argc, char *argv[])
{
    /**
     * Start parsing input parameters
     */
    std::string helpMessage = "\n\nThe Network Dependency Generator generates a dependency graph in the .dot file.\n"
                              "For each mode (partial,linear,bulk,deepeye) a .dot file will be generated.\n"
                              "(png) images can be generated using the 'dot' command in the terminal.\n"
                              "The bash script 'dot2png.sh' can be used for this.\n"
                              "To install dot: 'sudo apt-get install graphviz' .\n"
                              "Example of running dot: 'dot -Tpng AgeNet0-partial.dot -o AgeNet0-partial.png '\n"
                              "For more information about dot and Graphiz goto:\n"
                              "\thttps://renenyffenegger.ch/notes/tools/Graphviz/examples/index\n";

    cxxopts::Options options("ndg", "Network Dependency Generator" + helpMessage);
    options
        .positional_help("<key> <path>")
        .add_options()
            ("k,key", "Name of the network. For example 'AgeNet'", cxxopts::value<std::string>())
            ("p,path", "Path to the network. For example '../networks/Agenet'", cxxopts::value<std::string>())
            ("h,help", "Print help message")
            ;

    options.parse_positional({"key", "path"});
    auto result = options.parse(argc, argv);

    // Check if the help message needs to be printed;
    if (result.count("help") || result.arguments().empty() || !(result.count("key") && result.count("path")))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    std::string networkKey = result["key"].as<std::string>();
    std::string networkPath = result["path"].as<std::string>();
    /**
     * End parsing input parameters
     */

    // Init glog
    ::google::InitGoogleLogging(argv[0]);

    // Create arrival
    int delay = 0;
    EdgeCaffe::Arrival arr{delay, "", networkPath, networkKey};

    std::vector<std::pair<EdgeCaffe::Orchestrator::MODEL_SPLIT_MODE, std::string>> modes = {
            {EdgeCaffe::Orchestrator::PARTIAL, "partial"}
            ,{EdgeCaffe::Orchestrator::BULK, "bulk"}
            ,{EdgeCaffe::Orchestrator::DEEPEYE, "deepeye"}
            ,{EdgeCaffe::Orchestrator::LINEAR, "linear"}
    };

    // Iterate over all modes
    for(const auto& pair : modes)
    {
        // Setup orchestrator
        EdgeCaffe::Orchestrator orchestrator;
        auto mode = pair.first;
        auto modeAsString = pair.second;
        if (mode == EdgeCaffe::Orchestrator::BULK)
        {
            orchestrator.setupBulkMode();
        } else if (mode == EdgeCaffe::Orchestrator::DEEPEYE)
        {
            orchestrator.setupDeepEyeMode();
        } else if (mode == EdgeCaffe::Orchestrator::LINEAR)
        {
            orchestrator.setupLinearMode();
        } else
        { // Partial
            orchestrator.setupPartialMode(2);
        }
        orchestrator.splitMode = mode;
        orchestrator.splitModeAsString = modeAsString;

        // Submit arrival to generate the tasks
        orchestrator.submitInferenceTask(arr);

        // Lets create the output dot files
        for(auto iTask : orchestrator.inferenceTasks)
        {
            std::string dotContent = iTask->net->tasksToDotDebug();
            std::string _name = iTask->net->subTasks.front()->networkName + std::to_string(iTask->net->networkId) + "-" + modeAsString;
            std::string basePath = "../dot/";
            // Make sure that the output location exists
            std::filesystem::create_directories(basePath);
            std::string dotFileName = basePath +  _name + ".dot";
            std::filesystem::path dotPath = dotFileName;
            std::replace( dotFileName.begin(), dotFileName.end(), ' ', '_');
            // Write all to file
            std::ofstream fout(dotFileName, std::ios::out);
            fout << dotContent << std::endl;
            fout.close();
            std::cout << "Dot file written to "<< std::filesystem::canonical(dotPath) << std::endl;
        }
    }
}