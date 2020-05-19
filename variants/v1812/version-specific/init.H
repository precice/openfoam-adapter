std::cout << "Using a variant of the preciceAdapter for OpenFOAM v1812 or newer." << std::endl;

// Patch for issue #27: warning "MPI was already finalized" while
// running in serial. This only affects openfoam.com, while initNull()
// does not exist in openfoam.org.
UPstream::initNull();