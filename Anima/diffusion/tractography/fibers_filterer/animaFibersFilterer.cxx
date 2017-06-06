#include <tclap/CmdLine.h>

#include <animaReadWriteFunctions.h>
#include <animaFibersWriter.h>

#include <animaFibersReader.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkCleanPolyData.h>

#include <itkNearestNeighborInterpolateImageFunction.h>

void filterTracks(vtkPolyData *tracks, itk::NearestNeighborInterpolateImageFunction < itk::Image <unsigned short, 3> > * interpolator,
                  const std::vector <unsigned int> &touchLabels, const std::vector <unsigned int> &forbiddenLabels)
{
    vtkIdType numCells = tracks->GetNumberOfCells();

    std::vector <unsigned int> seenLabels;
    double pointPositionVTK[3];
    itk::ContinuousIndex<double, 3> currentIndex;
    typedef itk::Image <unsigned short, 3>::PointType PointType;
    PointType pointPosition;

    for (unsigned int i = 0;i < numCells;++i)
    {
        // Inspect i-th cell
        vtkCell *cell = tracks->GetCell(i);
        vtkPoints *cellPts = cell->GetPoints();
        vtkIdType numCellPts = cellPts->GetNumberOfPoints();
        seenLabels.clear();
        bool lineOk = true;

        for (unsigned int j = 0;j < numCellPts;++j)
        {
            cellPts->GetPoint(j, pointPositionVTK);
            for (unsigned int k = 0; k < 3; ++k)
                pointPosition[k] = pointPositionVTK[k];

            interpolator->GetInputImage()->TransformPhysicalPointToContinuousIndex(pointPosition,currentIndex);

            if (!interpolator->IsInsideBuffer(currentIndex))
                continue;

            unsigned int value = interpolator->EvaluateAtContinuousIndex(currentIndex);

            for (unsigned int k = 0;k < forbiddenLabels.size();++k)
            {
                if (value == forbiddenLabels[k])
                {
                    lineOk = false;
                    break;
                }
            }

            if (!lineOk)
                break;

            for (unsigned int k = 0;k < touchLabels.size();++k)
            {
                if (value == touchLabels[k])
                {
                    bool alreadyIn = false;
                    for (unsigned int l = 0;l < seenLabels.size();++l)
                    {
                        if (seenLabels[l] == value)
                        {
                            alreadyIn = true;
                            break;
                        }
                    }

                    if (!alreadyIn)
                        seenLabels.push_back(value);

                    break;
                }
            }
        }

        if (!lineOk || (seenLabels.size() != touchLabels.size()))
            tracks->DeleteCell(i);
    }

    tracks->RemoveDeletedCells();

    // Out of security, but apparently does not do much
    vtkSmartPointer <vtkCleanPolyData> vtkCleaner = vtkSmartPointer <vtkCleanPolyData>::New();
    vtkCleaner->SetInputData(tracks);
    vtkCleaner->Update();
    tracks->ShallowCopy(vtkCleaner->GetOutput());

    std::cout << "Kept " << tracks->GetNumberOfCells() << " after filtering" << std::endl;
}

int main(int argc, char **argv)
{
    TCLAP::CmdLine cmd("Filters fibers from a vtp file using a label image and specifying with several -t and -f which labels should be touched or are forbidden for each fiber. INRIA / IRISA - VisAGeS Team", ' ',ANIMA_VERSION);

    TCLAP::ValueArg<std::string> inArg("i","input","input tracks file",true,"","tracks vtp file",cmd);
    TCLAP::ValueArg<std::string> roiArg("r","roi","input ROI label image",true,"","ROI image",cmd);
    TCLAP::ValueArg<std::string> outArg("o","output","output tracks name",true,"","tracks",cmd);

    TCLAP::MultiArg<unsigned int> touchArg("t", "touch", "Labels that have to be touched",false,"touched labels",cmd);
    TCLAP::MultiArg<unsigned int> forbiddenArg("f", "forbid", "Labels that must not to be touched",false,"forbidden labels",cmd);

    try
    {
        cmd.parse(argc,argv);
    }
    catch (TCLAP::ArgException& e)
    {
        std::cerr << "Error: " << e.error() << "for argument " << e.argId() << std::endl;
        return EXIT_FAILURE;
    }

    typedef itk::Image <unsigned short, 3> ROIImageType;
    ROIImageType::Pointer roiImage = anima::readImage <ROIImageType> (roiArg.getValue());

    typedef itk::NearestNeighborInterpolateImageFunction <ROIImageType> InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    interpolator->SetInputImage(roiImage);

    anima::FibersReader trackReader;
    trackReader.SetFileName(inArg.getValue());
    trackReader.Update();

    vtkSmartPointer <vtkPolyData> tracks = trackReader.GetOutput();

    std::vector <unsigned int> touchLabels = touchArg.getValue();
    std::vector <unsigned int> forbiddenLabels = forbiddenArg.getValue();

    filterTracks(tracks,interpolator,touchLabels,forbiddenLabels);

    anima::FibersWriter writer;
    writer.SetInputData(tracks);
    writer.SetFileName(outArg.getValue());
    std::cout << "Writing tracks: " << outArg.getValue() << std::endl;
    writer.Update();

    return EXIT_SUCCESS;
}
