#include "computationthread.h"

#include <QTime>
#include <QDebug>
#include "interface/input_manager.h"
#include "math/simplex_tree.h"
#include "math/multi_betti.h"


ComputationThread::ComputationThread(int verbosity, QObject *parent) :
    QThread(parent),
    verbosity(verbosity)
{ }

ComputationThread::~ComputationThread()
{ }

void ComputationThread::compute(InputParameters& p, std::vector<double>& xg, std::vector<double>& yg, std::vector<xiPoint>& xi)
{
    params = p;
    x_grades = xg;
    y_grades = yg;
    xi_support = xi;

    //do I need to check anything here???

    start();
}

//this function does the work
void ComputationThread::run()
{
    QTime timer;    //for timing the computations

  //STEP 1: INPUT DATA AND CREATE BIFILTRATION

    //start the input manager
    InputManager im(params.dim, verbosity);
    //const char* filestr = fileName.toStdString().data();
    std::string filestr = params.fileName.toStdString();
    im.start(filestr, params.x_bins, params.y_bins);

    //get the data
    x_grades = im.get_x_grades();
    y_grades = im.get_y_grades();
    std::vector<exact> x_exact = im.get_x_exact();
    std::vector<exact> y_exact = im.get_y_exact();
    SimplexTree* bifiltration = im.get_bifiltration();

    //get data extents
    double data_xmin = x_grades.front();
    double data_xmax = x_grades.back();
    double data_ymin = y_grades.front();
    double data_ymax = y_grades.back();

    //print bifiltration statistics
    qDebug() << "\nBIFILTRATION:";
    qDebug() << "   Number of simplices of dimension" << params.dim << ":" << bifiltration->get_size(dim);
    qDebug() << "   Number of simplices of dimension" << (params.dim+1) << ":" << bifiltration->get_size(dim+1);
    qDebug() << "   Number of x-grades:" << x_grades.size() << "; values" << data_xmin << "to" << data_xmax;
    qDebug() << "   Number of y-grades:" << y_grades.size() << "; values" << data_ymin << "to" << data_ymax << "\n";

    if(verbosity >= 10)
    {
      qDebug() << "x-grades:";
      for(unsigned i=0; i<x_grades.size(); i++)
          qDebug() << "  " << x_grades[i] << " = " << x_exact[i];
      qDebug() << "y-grades:";
      for(unsigned i=0; i<y_grades.size(); i++)
          qDebug() << "  " << y_grades[i] << " = " << y_exact[i];
    }


  //STEP 2: COMPUTE SUPPORT POINTS OF MULTI-GRADED BETTI NUMBERS

    //compute xi_0 and xi_1 at all multi-grades
    if(verbosity >= 2) { qDebug() << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << dim << ":"; }
    MultiBetti mb(bifiltration, params.dim, verbosity);

    timer.start();
    mb.compute_fast();
    qDebug() << "  --> xi_i computation took" << timer.elapsed() << "milliseconds";


  //STEP 3: BUILD THE ARRANGEMENT
/*
    //build the arrangement
    if(verbosity >= 2) { qDebug() << "CALCULATING ANCHORS AND BUILDING THE DCEL ARRANGEMENT"; }

    TODO: need to figure out how arrangement gets passed here
    also need to figure out how to use signals and slots to send data from this thread back to the VisualizationWindow

    timer.start();
    arrangement = new Mesh(x_grades, x_exact, y_grades, y_exact, verbosity);
    arrangement->build_arrangement(mb, xi_support);     //also stores list of xi support points in the last argument
      //NOTE: this also computes and stores barcode templates in the arrangement

    qDebug() << "   building the line arrangement and computing all barcode templates took" << timer.elapsed() << "milliseconds";
  */
}//end run()