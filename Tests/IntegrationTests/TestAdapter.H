#ifndef PRECICETESTADAPTER_H
#define PRECICETESTADAPTER_H


#define CATCH_CONFIG_RUNNER

#include "fakeit.hpp"

#define private public

#include "Adapter.H"
using namespace fakeit;

// Other header files
// YAML reader - Used to read the adapter's configuration file.
#include "yaml-cpp/yaml.h"
#include <map>  

namespace preciceAdapter
{

class TestAdapter: public Adapter
{
       public:

        // Methods called by the functionObject

        //- Constructor
        TestAdapter(const Foam::Time& runTime, const Foam::fvMesh& mesh);

        //- Setup the adapter's configuration
        void testConfigure();

        //- Called by the functionObject's execute()
        void testExecute();

        //- Called by the functionObject's adjustTimeStep()
        void testAdjustSolverTimeStep();

        //- Called by the functionObject's end()
        void testEnd();

        //- Destructor
        ~TestAdapter();



        std::map<std::string, int> mesh_id = {
            { "Fluid-Mesh", 0 }
        };            

        std::map<std::string, int> data_id = {
            { "Heat-Flux", 1 },
            { "Temperature", 0 }
        };
    private:
        Mock<precice::SolverInterface> mock;
        int coupling_step = 1;
        int iteration_step = 1;

        std::map<int, int> implicit_inner_steps = {
            {1, 5},
            {2, 4},
            {3, 4},
            {4, 3},
            {5, 3}
        };
        // Size of data buffers required for reading and writing data
        int read_data_size = 161;
        int write_data_size = 161;
};

}

#endif
