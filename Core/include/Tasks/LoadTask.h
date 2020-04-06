//
// Created by bacox on 30/03/2020.
//

#ifndef PIPELINE_LOADTASK_H
#define PIPELINE_LOADTASK_H

#include "Task.h"
class LoadTask :public Task{
public:
    LoadTask(int id, int executionTime) : Task(id, executionTime) {}

    LoadTask() {}

    std::string partialName;
    std::string pathToPartial;

    void run() override;

    bool needsLoading = true;
};


#endif //PIPELINE_LOADTASK_H
