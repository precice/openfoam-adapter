This tutorial introduces an exemplary simulation setup for the utilization of a nearest-projection mapping with the openfoam-adapter. The demonstrated "flow-over-plate" case is exactly the same as in the already existing CHT tutorial. The following text explains the _general functionality_ of the adapter, the _current capability_ of the adapter and the necessary _changes in the tutorials_ for your own simulation. 

### General Information
Let's start with general information about mapping methods. The [preCICE wiki](https://github.com/precice/precice/wiki/Mapping-Configuration) contains an overview with available mapping methods in preCICE. The nearest-projection mapping is a second-order method. But contrary to the RBF mappings, connectivity information of the mesh are needed by preCICE, in order to interpolate e.g. between two nodes. 
There are two participants in each mapping, but the connectivity of one mesh is obviously sufficient for an interpolation. Which of the participants must provide mesh connectivity is not arbitrary and depends on the constraint type:

- for `consistent` mappings, the `from` participant needs to have connectivity 
- for `conservative` mappings, the `to` participant needs to have connectivity

Example: 
In our case, we exchange the `Temperature` data `consistent` from participant `Fluid` to `Solid`. Hence, the participant `Fluid` needs to provide mesh connectivity. 
Note: 

- In a standard **CHT** calculation, both data sets (heat-flux and temperature) are mapped consistent. Therefore, both participants need to provide connectivity.
- In a standard **FSI** calculation, forces are mapped conservative, while displacements are mapped consistent. Consequently, the Solid participant needs to provide connectivity and not the Fluid participant.

If mesh connectivity is not provided in the described way, you are nevertheless able to define a nearest-projection mapping in your `precice-config.xml`file, but it will _fall back to an expensive first order nearest-neighbor mapping_.

### Adapter Implementation
Since OpenFOAM is a finite-volume based solver, data is located in the middle of the cell /on the cell face centers for a coupling interface. For connectivity, preCICE has the functions `setMeshTriangle` and `setMeshEdge` or a combination. Using the face centers as basis for this functions is cumbersome. The main reason is, that OpenFOAM distributes your mesh in parallel to each processor, but connectivity needs to be defined over the partitioned mesh boundaries. This problem vanishes, if we define mesh connectivity on face nodes, since one node is part of both processors (at the decomposition boundary) in a decomposed mesh. Therefore, mesh connectivity can only be provided on the face nodes not on the face centers. 
As described, the data location is not the face node, but the face center. Therefore, we make use of OpenFOAM functions to interpolate from face centers to face nodes. The following image illustrates the workflow:
![nearest-projection](https://user-images.githubusercontent.com/33414590/55965109-3402b600-5c76-11e9-87eb-0cdb10b55f7b.png)
Data is obtained at the face centers, then interpolated to face nodes. Here, we have provided mesh connectivity and finally, preCICE performs the nearest-projection mapping. 
It is important to notice, that the target data location is again the face center mesh of the coupling partner. In the standard CHT case, where both data sets are exchanged by a nearest-projection mapping, this leads to four interface meshes (two (centers and nodes) per participant). There is one simple reason: Therefore, we could skip one interpolation (namely OpenFOAM intern from nodes to centers on the target mesh (cf. picture solver B)). 

### Capability of the Implementation
As already mentioned, the `Fluid`participant doesn't need to provide the mesh connectivity in case of a standard FSI. Therefore, the `Solid`participant needs to provide it and nothing special needs to be considered compared to other mapping methods.
This implementation supports all CHT related data exchanges, which are mapped with a `consistent`mapping constraint. The required settings and differences compared to the existing tutorial are given below.

### Changes in the Simulation Setup
Changes in the `precice-config.xml` file:
In order to map from face nodes to face centers, both meshes need to be specified. The node based mesh uses the write data and the center based mesh uses the read data. Have a look in the given `precice-config.xml` in this tutorial. Example: `Temperature` is calculated by the `Fluid` participant and passed to the `Solid` participant. Therefore, it is the write data of the participant `Fluid` and the read data of the participant `Solid`. This results in the following two meshes for this data:
```
 <mesh name="Fluid-Mesh-Nodes">
     <use-data name="Temperature"/>
  </mesh>
  <mesh name="Solid-Mesh-Centers">
     <use-data name="Temperature"/>
  </mesh>
```
All further changes follow from this interface splitting. Have a look in the given config files for all details.

Changes in the solver specific `yml` files:
By default, the OpenFOAM adapter doesn't provide any connectivity information. Therefore, a new boolean variable called `provideMeshConnectivity`is introduced. This variable is associated to each interface and can be set accordingly. Note: The connectivity can only be provided in case of the `locationsType: faceNodes` (see section Adapter Implementation above for details). Similar to the interface splitting in the `precice-config.xml` file, the interfaces need to be defined in the `yml` file. Examples are given in the tutorial solver and look the following way:
```
interfaces:
- mesh: Fluid-Mesh-Centers
  locations: faceCenters # optional, defaults to faceCenters
  provideMeshConnectivity: false
  patches:
  - interface
  read-data: Heat-Flux
- mesh: Fluid-Mesh-Nodes
  locations: faceNodes # optional, defaults to faceCenters
  provideMeshConnectivity: true
  patches:
  - interface
  write-data: Temperature
```
The participant `Fluid` has its read data `Heat-Flux`, which lives on the `faceCenters`, and its write data `Temperature`, which lives on the `faceNodes`. The mesh connectivity needs only to be provided in case of the `faceNodes` by using the boolean `provideMeshConnectivity: true`.

### General Notes
Since you provide now mesh connectivity on your interface, you can export your coupling interface with the tag `<export: vtk ...` in your `precice-config.xml`. Make sure you have created the specified target directory (preCICE-output in the example files) in your local simulation directory, otherwise preCICE will complain about it. The mesh will be triangulated, although you use hexahedral meshes. This has nothing to do with your mesh and is just caused by the way the connectivity is defined in preCICE. As described above, the function `setMeshTriangles` is used to define the connectivity. Hence, every interface cell/face is represented by two triangles. The following image should give you an impression of a possible triangulated coupling mesh, which consists purely of hexahedral cells:
![triangulated](https://user-images.githubusercontent.com/33414590/55974257-96b07d80-5c87-11e9-9965-972b922c483d.png)
 
Notes on 2D Cases
The geometry of the tutorial differs compared to the existing example: The out-of-plane thickness of the domain is reduced clearly and it is recommended. Otherwise your face centes have a quite large distance to the face nodes, which might trigger a preCICE error message, which complains about this fact. Your error message will filter out one of the meshes, especially in parallel simulations.  

 
