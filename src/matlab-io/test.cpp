
#include "test.hpp"

#include <matlab-io/MatlabIO.hpp>

void matlab::test()
{
  QString matFile = "C:/Daniel/facu/ENGN2502-2016/3dp-course-2016/src/matlab-io/test.mat";
  //QString matFile = "D:/data/turntable/01/Calib_Results.mat";

  // create a new reader
  MatlabIO matio;
  bool ok = matio.open(matFile, "r");
  if (ok)
  {
    // read all of the variables in the file
    auto varMap = matio.read();

    // close the file
    matio.close();

    // display the file info
    matio.whos(varMap);
  }
}

