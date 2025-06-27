#include "include/core/Data_subclass/all_dataHead.h"
#include "include/core/Distance_subclass/all_distanceHead.h"
#include "include/utils/MetricSpaceSearch.h"
#include "include/utils/Solution.h"
#include "include/utils/LoadData.h"
#include <iostream>

using namespace std;

int main() {
    try {
        int feedback = systemLaunch();
        
    }
    catch (const exception& e) {
        cout << "´íÎó: " << e.what() << endl;
        return 1;
    }

    return 0;
}