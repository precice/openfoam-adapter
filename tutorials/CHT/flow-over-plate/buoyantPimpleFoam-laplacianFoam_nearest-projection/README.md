This tutorial introduces an example simulation setup for nearest-projection mapping with the OpenFOAM adapter. The demonstrated "flow over a heated plate" scenario is exactly the same as in the `buoyantPimpleFoam-laplacianFoam` tutorial (with a thinner plate). The following text explains the _general functionality_ of the adapter, the _current capability_ of the adapter and the necessary _changes in the tutorials_ for your own simulation. 

## General Information
Let's start with general information about mapping methods. The [preCICE wiki](https://github.com/precice/precice/wiki/Mapping-Configuration) contains an overview with available mapping methods in preCICE. The nearest-projection mapping is a second-order method. But, contrary to the RBF mapping, mesh connectivity information is needed for the interpolation. 
There are two participants in each mapping, but the connectivity of one mesh is sufficient for an interpolation. Which of the participants must provide mesh connectivity is not arbitrary and depends on the constraint type:

- for `consistent` mappings, the `from` participant needs to provide connectivity 
- for `conservative` mappings, the `to` participant needs to provide connectivity

For example, in our case, we exchange `Temperature` data in a `consistent` way from the `Fluid` to the `Solid` participant. Hence, `Fluid` needs to provide mesh connectivity.

Notes: 

- In a standard **CHT** calculation, both data sets (heat flux and temperature) are mapped consistently. Therefore, both participants need to provide connectivity.
- In a standard **FSI** calculation, forces are mapped conservatively from `Fluid` to `Solid`, while displacements are mapped consistently from `Solid` to `Fluid`. Hence, it's `Solid` that needs to provide connectivity and not `Fluid`.

If mesh connectivity is not provided in the described way, you are nevertheless able to define a nearest-projection mapping in your `precice-config.xml`file, but it will _fall back to a first order nearest-neighbor mapping_.

## Adapter Implementation
Since OpenFOAM is a finite-volume based solver, data is located in the middle of the cell, or on the cell face centers for a coupling interface. Mesh connectivity can be given to preCICE using the methods `setMeshTriangle` and `setMeshEdge`. Using the face centers as arguments for these methods is cumbersome. The main reason is that, although OpenFOAM decomposes the mesh for parallel simulations and distributes the subdomains to different processes, mesh connectivity needs to be defined over the partitioned mesh boundaries. This problem vanishes if we define mesh connectivity based on the face nodes, since boundary nodes can be shared among processors. Therefore, mesh connectivity can only be provided on the face nodes not on the face centers.

As described already, the data is not stored on the face nodes, but on the face centers. Therefore, we use OpenFOAM functions to interpolate from face centers to face nodes. The following image illustrates the workflow:

![nearest-projection](https://user-images.githubusercontent.com/33414590/55965109-3402b600-5c76-11e9-87eb-0cdb10b55f7b.png)

Data is obtained at the face centers, then interpolated to face nodes. Here, we have provided mesh connectivity and finally, preCICE performs the nearest-projection mapping. 
It is important to notice that the target data location is again the face center mesh of the coupling partner. In the standard CHT case, where both data sets are exchanged by a nearest-projection mapping, this leads to two interface meshes (centers and nodes) per participant. Having both the centers and nodes defined, we can skip one interpolation step and read data directly to the centers (cf. picture solver B). 

## Supported fields
As already mentioned, the `Fluid` participant does not need to provide the mesh connectivity in case of a standard FSI. Therefore, the `Solid` participant needs to provide it and nothing special needs to be considered compared to other mapping methods.
This implementation supports all CHT-related fields, which are mapped with a `consistent` constraint. The required settings and differences compared to the basic tutorial are given below.

## Changes in the Simulation Setup

As we are defining two meshes for each participant, we need to define them in the `precice-config.xml` and `precice-adapter-config.yml` configuration files. Additionally, we need to enable the `provideMeshConnectivity` switch for the adapter.

### Changes in `precice-config.xml`
In order to map from face nodes to face centers, both meshes need to be specified. The nodes-based mesh uses the write data and the centers-based mesh uses the read data. Have a look in the given `precice-config.xml` in this tutorial. Example: `Temperature` is calculated by the `Fluid` participant and passed to the `Solid` participant. Therefore, it is the write data of the participant `Fluid` and the read data of the participant `Solid`. This results in the following two meshes for this data:
```
<mesh name="Fluid-Mesh-Nodes">
  <use-data name="Temperature"/>
</mesh>
<mesh name="Solid-Mesh-Centers">
  <use-data name="Temperature"/>
</mesh>
```
All further changes follow from this interface splitting. Have a look in the given config files for all details.

### Changes in `precice-adapter-config.yml`

By default, the OpenFOAM adapter doesn't provide any connectivity information. Therefore, a new boolean variable called `provideMeshConnectivity` is introduced. This variable is associated to each interface and can be set accordingly. Note: Mesh connectivity can only be provided in case `locationsType: faceNodes` is set (see section Adapter Implementation). Similar to the interface splitting in the `precice-config.xml` file, the interfaces also need to be defined in the `yml` file. For example:

```
interfaces:
- mesh: Fluid-Mesh-Centers
  locations: faceCenters         # default
  provideMeshConnectivity: false # default
  patches:
  - interface
  read-data: Heat-Flux
- mesh: Fluid-Mesh-Nodes
  locations: faceNodes
  provideMeshConnectivity: true
  patches:
  - interface
  write-data: Temperature
```
The participant `Fluid` has its read data `Heat-Flux`, which is read on the `faceCenters`, and its write data `Temperature`, which is written to the `faceNodes`. The mesh connectivity needs only to be provided in case of the `faceNodes`, using the option `provideMeshConnectivity: true`.

## General Notes

Since you now define mesh connectivity on your interface, you can export your coupling interface with the tag `<export:vtk directory="preCICE-output" />` in your `precice-config.xml`. Make sure you have created the specified target directory (preCICE-output in the example files) in your local simulation directory, otherwise preCICE will complain about it.

Visualizing these files (e.g. using ParaView) will show a triangular mesh, even though you use hexahedral meshes. This has nothing to do with your mesh and is just caused by the way the connectivity is defined in preCICE. As described above, the function `setMeshTriangles` is used to define the connectivity. Hence, every interface cell/face is represented by two triangles. The following image should give you an impression of a possible triangulated coupling mesh, which consists purely of hexahedral cells:

![triangulated](https://user-images.githubusercontent.com/33414590/55974257-96b07d80-5c87-11e9-9965-972b922c483d.png)
 
Note: Connectivity is defined on meshes associated with mesh nodes, which are named respectively e.g. Fluid-Mesh-Nodes. In this case, you could directly see the interface without applying filters by loading the `.vtk` files. In order to visualize additionally center based meshes, where no connectivity is provided, select a Glyph filter in paraView. Furthermore, it makes a difference, on which participant the `<export...` tag is defined in your `precice-config.xml` file. Each participant exports interface meshes, which he provides or receives. The receiving participant filters out mesh parts that it does not need (for the mapping). Hence, a received mesh might look incomplete.

### Notes on 2D Cases

The geometry of the tutorial differs compared to the existing example: The out-of-plane thickness of the domain is reduced clearly and it is recommended. Otherwise your face centes have a quite large distance to the face nodes, which might trigger a preCICE warning. In that case, preCICE may filter out one of the meshes, especially in parallel simulations.  

### Disclaimer

This offering is not approved or endorsed by OpenCFD Limited, producer and distributor of the OpenFOAM software via www.openfoam.com, and owner of the OPENFOAM® and OpenCFD® trade marks.
