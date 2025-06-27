#pragma once
#include "../../include/core/Data_subclass/all_dataHead.h"
#include "../../include/core/Distance_subclass/all_distanceHead.h"
#include "../../include/utils/Solution.h"
#include "../../include/utils/MetricSpaceSearch.h"
#include "../../include/utils/LoadData.h"
#include "../../include/metric_search/all_search.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <stdexcept>

using namespace std;

//启动主系统，为了main函数简洁，我把他们独立出来了
int systemLaunch();

