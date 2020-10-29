#!/usr/bin/env python

###
### This file is generated automatically by SALOME v9.3.0 with dump python functionality
###

import sys
import salome

salome.salome_init()
import salome_notebook
notebook = salome_notebook.NoteBook()
sys.path.insert(0, r'/home/tenzinger/work/of/2020_heat_water/CAD')

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS


geompy = geomBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Vertex_1 = geompy.MakeVertex(17, 0, -32.5)
Vertex_2 = geompy.MakeVertex(17, 0, -42.5)
Vertex_3 = geompy.MakeVertex(-17, 0, -42.5)
Vertex_4 = geompy.MakeVertex(-17, 0, -32.5)
Circle_1 = geompy.MakeCircle(Vertex_1, OY, 4.25)
Circle_2 = geompy.MakeCircle(Vertex_2, OY, 4.25)
Circle_3 = geompy.MakeCircle(Vertex_3, OY, 4.25)
Circle_4 = geompy.MakeCircle(Vertex_4, OY, 4.25)

Vertex_5 = geompy.MakeVertex(75, 0, -75)
Vertex_6 = geompy.MakeVertex(75, 0, 100)
Vertex_7 = geompy.MakeVertex(-75, 0, 100)
Vertex_8 = geompy.MakeVertex(-75, 0, -75)
Line_1 = geompy.MakeLineTwoPnt(Vertex_6, Vertex_7)
Line_2 = geompy.MakeLineTwoPnt(Vertex_7, Vertex_8)
Line_3 = geompy.MakeLineTwoPnt(Vertex_8, Vertex_5)
Line_1_vertex_2 = geompy.GetSubShape(Line_1, [2])
Line_4 = geompy.MakeLineTwoPnt(Vertex_5, Line_1_vertex_2)

Face_1 = geompy.MakeFaceWires([Circle_1, Circle_2, Circle_3, Circle_4, Line_1, Line_2, Line_3, Line_4], 1)
Face_2 = geompy.MakeFaceWires([Circle_1], 1)
Face_3 = geompy.MakeFaceWires([Circle_2], 1)
Face_4 = geompy.MakeFaceWires([Circle_3], 1)
Face_5 = geompy.MakeFaceWires([Circle_4], 1)

# fluid
Extrusion_1 = geompy.MakePrismVecH2Ways(Face_1, OY, 0.5)
[Face_6,Face_7,Face_8,Face_9,Face_10,Face_11,Face_12,Face_13,Face_14,Face_15] = geompy.ExtractShapes(Extrusion_1, geompy.ShapeType["FACE"], True)
[Edge_1,Edge_2,Edge_3,Edge_4,Edge_5,Edge_6,Edge_7,Edge_8,Edge_9,Edge_10,Edge_11,Edge_12,Edge_13,Edge_14,Edge_15,Edge_16,Edge_17,Edge_18,Edge_19,Edge_20,Edge_21,Edge_22,Edge_23,Edge_24] = geompy.ExtractShapes(Extrusion_1, geompy.ShapeType["EDGE"], True)

# heater 1
Extrusion_2 = geompy.MakePrismVecH2Ways(Face_2, OY, 0.5)
[Face_16,Face_17,Face_18] = geompy.ExtractShapes(Extrusion_2, geompy.ShapeType["FACE"], True)
[Edge_25,Edge_26,Edge_27] = geompy.ExtractShapes(Extrusion_2, geompy.ShapeType["EDGE"], True)

# heater 2
Extrusion_3 = geompy.MakePrismVecH2Ways(Face_3, OY, 0.5)
[Face_19,Face_20,Face_21] = geompy.ExtractShapes(Extrusion_3, geompy.ShapeType["FACE"], True)
[Edge_28,Edge_29,Edge_30] = geompy.ExtractShapes(Extrusion_3, geompy.ShapeType["EDGE"], True)

# heater 3
Extrusion_4 = geompy.MakePrismVecH2Ways(Face_4, OY, 0.5)
[Face_22,Face_23,Face_24] = geompy.ExtractShapes(Extrusion_4, geompy.ShapeType["FACE"], True)
[Edge_31,Edge_32,Edge_33] = geompy.ExtractShapes(Extrusion_4, geompy.ShapeType["EDGE"], True)

# heater 4
Extrusion_5 = geompy.MakePrismVecH2Ways(Face_5, OY, 0.5)
[Face_25,Face_26,Face_27] = geompy.ExtractShapes(Extrusion_5, geompy.ShapeType["FACE"], True)
[Edge_34,Edge_35,Edge_36] = geompy.ExtractShapes(Extrusion_5, geompy.ShapeType["EDGE"], True)

Auto_group_for_Sub_mesh_5 = geompy.CreateGroup(Extrusion_1, geompy.ShapeType["EDGE"])
geompy.UnionList(Auto_group_for_Sub_mesh_5, [Edge_1, Edge_4, Edge_7, Edge_8, Edge_17, Edge_18, Edge_21, Edge_24])
Auto_group_for_Sub_mesh_6 = geompy.CreateGroup(Extrusion_1, geompy.ShapeType["EDGE"])
geompy.UnionList(Auto_group_for_Sub_mesh_6, [Edge_5, Edge_6, Edge_9, Edge_10, Edge_15, Edge_16, Edge_19, Edge_20])

Vertex_9 = geompy.MakeVertexWithRef(Vertex_1, 0, 0, -5)
Vertex_10 = geompy.MakeVertexWithRef(Vertex_4, 0, 0, -5)
Cylinder_1 = geompy.MakeCylinder(Vertex_9, OY, 2, 1)
Cylinder_2 = geompy.MakeCylinder(Vertex_10, OY, 2, 1)
Translation_1_1 = geompy.MakeTranslation(Cylinder_1, 0, -0.5, 0)
Translation_1_2 = geompy.MakeTranslation(Cylinder_2, 0, -0.5, 0)

Circle_01 = geompy.MakeCylinder(Vertex_1, OY, 8.0, 1)
Circle_02 = geompy.MakeCylinder(Vertex_2, OY, 8.0, 1)
Circle_03 = geompy.MakeCylinder(Vertex_3, OY, 8.0, 1)
Circle_04 = geompy.MakeCylinder(Vertex_4, OY, 8.0, 1)
Translation_2_1 = geompy.MakeTranslation(Circle_01, 0, -0.5, 0)
Translation_2_2 = geompy.MakeTranslation(Circle_02, 0, -0.5, 0)
Translation_2_3 = geompy.MakeTranslation(Circle_03, 0, -0.5, 0)
Translation_2_4 = geompy.MakeTranslation(Circle_04, 0, -0.5, 0)

Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 20)
Translation_1 = geompy.MakeTranslation(Box_1, -100, -100, -10)
Box_2 = geompy.MakeBoxDXDYDZ(80, 200, 45)
Translation_2 = geompy.MakeTranslation(Box_2, -40, -100, (-10-45))
Box_3 = geompy.MakeBoxDXDYDZ(100, 200, 30)
Translation_3 = geompy.MakeTranslation(Box_3, -50, -100, 10)


geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Vertex_4, 'Vertex_4' )
geompy.addToStudy( Circle_1, 'Circle_1' )
geompy.addToStudy( Circle_2, 'Circle_2' )
geompy.addToStudy( Circle_3, 'Circle_3' )
geompy.addToStudy( Circle_4, 'Circle_4' )
geompy.addToStudy( Vertex_5, 'Vertex_5' )
geompy.addToStudy( Vertex_6, 'Vertex_6' )
geompy.addToStudy( Vertex_7, 'Vertex_7' )
geompy.addToStudy( Vertex_8, 'Vertex_8' )
geompy.addToStudy( Line_1, 'Line_1' )
geompy.addToStudy( Line_2, 'Line_2' )
geompy.addToStudy( Line_3, 'Line_3' )
geompy.addToStudyInFather( Line_1, Line_1_vertex_2, 'Line_1:vertex_2' )
geompy.addToStudy( Line_4, 'Line_4' )
geompy.addToStudy( Face_2, 'Face_2' )
geompy.addToStudy( Face_3, 'Face_3' )
geompy.addToStudy( Face_4, 'Face_4' )
geompy.addToStudy( Face_5, 'Face_5' )
geompy.addToStudy( Face_1, 'Face_1' )
geompy.addToStudy( Extrusion_1, 'Extrusion_1' )
geompy.addToStudy( Extrusion_2, 'Extrusion_2' )
geompy.addToStudy( Extrusion_3, 'Extrusion_3' )
geompy.addToStudy( Extrusion_4, 'Extrusion_4' )
geompy.addToStudy( Extrusion_5, 'Extrusion_5' )
geompy.addToStudyInFather( Extrusion_1, Face_6, 'Face_6' )
geompy.addToStudyInFather( Extrusion_1, Face_7, 'Face_7' )
geompy.addToStudyInFather( Extrusion_1, Face_8, 'Face_8' )
geompy.addToStudyInFather( Extrusion_1, Face_9, 'Face_9' )
geompy.addToStudyInFather( Extrusion_1, Face_10, 'Face_10' )
geompy.addToStudyInFather( Extrusion_1, Face_11, 'Face_11' )
geompy.addToStudyInFather( Extrusion_1, Face_12, 'Face_12' )
geompy.addToStudyInFather( Extrusion_1, Face_13, 'Face_13' )
geompy.addToStudyInFather( Extrusion_1, Face_14, 'Face_14' )
geompy.addToStudyInFather( Extrusion_1, Face_15, 'Face_15' )
geompy.addToStudyInFather( Extrusion_2, Face_16, 'Face_16' )
geompy.addToStudyInFather( Extrusion_2, Face_17, 'Face_17' )
geompy.addToStudyInFather( Extrusion_2, Face_18, 'Face_18' )
geompy.addToStudyInFather( Extrusion_3, Face_19, 'Face_19' )
geompy.addToStudyInFather( Extrusion_3, Face_20, 'Face_20' )
geompy.addToStudyInFather( Extrusion_3, Face_21, 'Face_21' )
geompy.addToStudyInFather( Extrusion_4, Face_22, 'Face_22' )
geompy.addToStudyInFather( Extrusion_4, Face_23, 'Face_23' )
geompy.addToStudyInFather( Extrusion_4, Face_24, 'Face_24' )
geompy.addToStudyInFather( Extrusion_5, Face_25, 'Face_25' )
geompy.addToStudyInFather( Extrusion_5, Face_26, 'Face_26' )
geompy.addToStudyInFather( Extrusion_5, Face_27, 'Face_27' )
geompy.addToStudyInFather( Extrusion_1, Edge_1, 'Edge_1' )
geompy.addToStudyInFather( Extrusion_1, Edge_2, 'Edge_2' )
geompy.addToStudyInFather( Extrusion_1, Edge_3, 'Edge_3' )
geompy.addToStudyInFather( Extrusion_1, Edge_4, 'Edge_4' )
geompy.addToStudyInFather( Extrusion_1, Edge_5, 'Edge_5' )
geompy.addToStudyInFather( Extrusion_1, Edge_6, 'Edge_6' )
geompy.addToStudyInFather( Extrusion_1, Edge_7, 'Edge_7' )
geompy.addToStudyInFather( Extrusion_1, Edge_8, 'Edge_8' )
geompy.addToStudyInFather( Extrusion_1, Edge_9, 'Edge_9' )
geompy.addToStudyInFather( Extrusion_1, Edge_10, 'Edge_10' )
geompy.addToStudyInFather( Extrusion_1, Edge_11, 'Edge_11' )
geompy.addToStudyInFather( Extrusion_1, Edge_12, 'Edge_12' )
geompy.addToStudyInFather( Extrusion_1, Edge_13, 'Edge_13' )
geompy.addToStudyInFather( Extrusion_1, Edge_14, 'Edge_14' )
geompy.addToStudyInFather( Extrusion_1, Edge_15, 'Edge_15' )
geompy.addToStudyInFather( Extrusion_1, Edge_16, 'Edge_16' )
geompy.addToStudyInFather( Extrusion_1, Edge_17, 'Edge_17' )
geompy.addToStudyInFather( Extrusion_1, Edge_18, 'Edge_18' )
geompy.addToStudyInFather( Extrusion_1, Edge_19, 'Edge_19' )
geompy.addToStudyInFather( Extrusion_1, Edge_20, 'Edge_20' )
geompy.addToStudyInFather( Extrusion_1, Edge_21, 'Edge_21' )
geompy.addToStudyInFather( Extrusion_1, Edge_22, 'Edge_22' )
geompy.addToStudyInFather( Extrusion_1, Edge_23, 'Edge_23' )
geompy.addToStudyInFather( Extrusion_1, Edge_24, 'Edge_24' )
geompy.addToStudyInFather( Extrusion_2, Edge_25, 'Edge_25' )
geompy.addToStudyInFather( Extrusion_2, Edge_26, 'Edge_26' )
geompy.addToStudyInFather( Extrusion_2, Edge_27, 'Edge_27' )
geompy.addToStudyInFather( Extrusion_3, Edge_28, 'Edge_28' )
geompy.addToStudyInFather( Extrusion_3, Edge_29, 'Edge_29' )
geompy.addToStudyInFather( Extrusion_3, Edge_30, 'Edge_30' )
geompy.addToStudyInFather( Extrusion_4, Edge_31, 'Edge_31' )
geompy.addToStudyInFather( Extrusion_4, Edge_32, 'Edge_32' )
geompy.addToStudyInFather( Extrusion_4, Edge_33, 'Edge_33' )
geompy.addToStudyInFather( Extrusion_5, Edge_34, 'Edge_34' )
geompy.addToStudyInFather( Extrusion_5, Edge_35, 'Edge_35' )
geompy.addToStudyInFather( Extrusion_5, Edge_36, 'Edge_36' )
geompy.addToStudyInFather( Extrusion_1, Auto_group_for_Sub_mesh_5, 'Auto_group_for_Sub-mesh_5' )
geompy.addToStudyInFather( Extrusion_1, Auto_group_for_Sub_mesh_6, 'Auto_group_for_Sub-mesh_6' )
geompy.addToStudy( Vertex_9, 'Vertex_9' )
geompy.addToStudy( Vertex_10, 'Vertex_10' )
geompy.addToStudy( Cylinder_1, 'Cylinder_1' )
geompy.addToStudy( Cylinder_2, 'Cylinder_2' )
geompy.addToStudy( Translation_1_1, 'Translation_1_1' )
geompy.addToStudy( Translation_1_2, 'Translation_1_2' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Box_2, 'Box_2' )
geompy.addToStudy( Box_3, 'Box_3' )
geompy.addToStudy( Translation_1, 'Translation_1' )
geompy.addToStudy( Translation_2, 'Translation_2' )
geompy.addToStudy( Translation_3, 'Translation_3' )
geompy.addToStudy( Circle_01, 'Circle_01' )
geompy.addToStudy( Circle_02, 'Circle_02' )
geompy.addToStudy( Circle_03, 'Circle_03' )
geompy.addToStudy( Circle_04, 'Circle_04' )
geompy.addToStudy( Translation_2_1, 'Translation_2_1' )
geompy.addToStudy( Translation_2_2, 'Translation_2_2' )
geompy.addToStudy( Translation_2_3, 'Translation_2_3' )
geompy.addToStudy( Translation_2_4, 'Translation_2_4' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
#smesh.SetEnablePublish( False ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                 # multiples meshes built in parallel, complex and numerous mesh edition (performance)


#
Prism_3D = smesh.CreateHypothesis('Prism_3D')
Regular_1D = smesh.CreateHypothesis('Regular_1D')

NETGEN_2D = smesh.CreateHypothesis('NETGEN_2D_ONLY', 'NETGENEngine')

Number_of_Segments_1 = smesh.CreateHypothesis('NumberOfSegments')
Number_of_Segments_2 = smesh.CreateHypothesis('NumberOfSegments')

Number_of_Segments_1.SetNumberOfSegments( 90 )
Number_of_Segments_2.SetNumberOfSegments( 1 )

Local_Length_1 = smesh.CreateHypothesis('LocalLength')
Local_Length_2 = smesh.CreateHypothesis('LocalLength')

Local_Length_1.SetLength( 0.15 )
Local_Length_1.SetPrecision( 1e-07 )

Local_Length_2.SetLength( 2 )
Local_Length_2.SetPrecision( 1e-07 )

Viscous_Layers_2D_1 = smesh.CreateHypothesis('ViscousLayers2D')
Viscous_Layers_2D_1.SetTotalThickness( 2 )
Viscous_Layers_2D_1.SetNumberLayers( 8 )
Viscous_Layers_2D_1.SetStretchFactor( 1.15 )
Viscous_Layers_2D_1.SetEdges( [ 19, 26, 30 ], 0 )

Viscous_Layers_2D_2 = smesh.CreateHypothesis('ViscousLayers2D')
Viscous_Layers_2D_2.SetTotalThickness( 0.55 )
Viscous_Layers_2D_2.SetNumberLayers( 5 )
Viscous_Layers_2D_2.SetStretchFactor( 1.15 )
Viscous_Layers_2D_2.SetEdges( [ 51, 58, 44, 37 ], 0 )

Viscous_Layers_2D_3 = smesh.CreateHypothesis('ViscousLayers2D')
Viscous_Layers_2D_3.SetTotalThickness( 1.0 )
Viscous_Layers_2D_3.SetNumberLayers( 8 )
Viscous_Layers_2D_3.SetStretchFactor( 1.0 )

NETGEN_2D_Parameters_3 = smesh.CreateHypothesis('NETGEN_Parameters_2D_ONLY', 'NETGENEngine')
NETGEN_2D_Parameters_3.SetMaxSize( 2 )
NETGEN_2D_Parameters_3.SetMinSize( 0.15 )
NETGEN_2D_Parameters_3.SetGrowthRate( 0.1 )
NETGEN_2D_Parameters_3.SetOptimize( 1 )
NETGEN_2D_Parameters_3.SetFineness( 5 )
NETGEN_2D_Parameters_3.SetChordalError( -1 )
NETGEN_2D_Parameters_3.SetChordalErrorEnabled( 0 )
NETGEN_2D_Parameters_3.SetUseSurfaceCurvature( 1 )
NETGEN_2D_Parameters_3.SetWorstElemMeasure( 21880 )
NETGEN_2D_Parameters_3.SetUseDelauney( 0 )
NETGEN_2D_Parameters_3.SetQuadAllowed( 1 )
NETGEN_2D_Parameters_3.SetCheckChartBoundary( 85 )

NETGEN_2D_Parameters_3.SetLocalSizeOnShape(Translation_1_1, 0.15)
NETGEN_2D_Parameters_3.SetLocalSizeOnShape(Translation_1_2, 0.15)

NETGEN_2D_Parameters_3.SetLocalSizeOnShape(Translation_1, 1)
NETGEN_2D_Parameters_3.SetLocalSizeOnShape(Translation_2, 0.4)
NETGEN_2D_Parameters_3.SetLocalSizeOnShape(Translation_3, 1)

NETGEN_2D_Parameters_3.SetLocalSizeOnShape(Translation_2_1, 0.25)
NETGEN_2D_Parameters_3.SetLocalSizeOnShape(Translation_2_2, 0.25)
NETGEN_2D_Parameters_3.SetLocalSizeOnShape(Translation_2_3, 0.25)
NETGEN_2D_Parameters_3.SetLocalSizeOnShape(Translation_2_4, 0.25)

NETGEN_2D_Parameters_1 = smesh.CreateHypothesis('NETGEN_Parameters_2D_ONLY', 'NETGENEngine')
NETGEN_2D_Parameters_1.SetMaxSize( 2 )
NETGEN_2D_Parameters_1.SetMinSize( 0.15 )
NETGEN_2D_Parameters_1.SetGrowthRate( 0.15 )
NETGEN_2D_Parameters_1.SetOptimize( 1 )
NETGEN_2D_Parameters_1.SetFineness( 5 )
NETGEN_2D_Parameters_1.SetChordalError( -1 )
NETGEN_2D_Parameters_1.SetChordalErrorEnabled( 0 )
NETGEN_2D_Parameters_1.SetUseSurfaceCurvature( 1 )
NETGEN_2D_Parameters_1.SetWorstElemMeasure( 21880 )
NETGEN_2D_Parameters_1.SetUseDelauney( 0 )
NETGEN_2D_Parameters_1.SetQuadAllowed( 1 )
NETGEN_2D_Parameters_1.SetCheckChartBoundary( 85 )

#
Mesh_1 = smesh.Mesh(Extrusion_2)
Mesh_2 = smesh.Mesh(Extrusion_3)
Mesh_3 = smesh.Mesh(Extrusion_4)
Mesh_4 = smesh.Mesh(Extrusion_5)
Mesh_5 = smesh.Mesh(Extrusion_1)

Sub_mesh_1 = Mesh_1.GetSubMesh( Edge_26, 'Sub-mesh_1' )
Sub_mesh_2 = Mesh_2.GetSubMesh( Edge_29, 'Sub-mesh_2' )
Sub_mesh_3 = Mesh_3.GetSubMesh( Edge_32, 'Sub-mesh_3' )
Sub_mesh_4 = Mesh_4.GetSubMesh( Edge_35, 'Sub-mesh_4' )

Sub_mesh_11 = Mesh_1.GetSubMesh( Face_18, 'Sub-mesh_11' )
Sub_mesh_12 = Mesh_2.GetSubMesh( Face_21, 'Sub-mesh_12' )
Sub_mesh_13 = Mesh_3.GetSubMesh( Face_24, 'Sub-mesh_13' )
Sub_mesh_14 = Mesh_4.GetSubMesh( Face_27, 'Sub-mesh_14' )

Sub_mesh_5 = Mesh_5.GetSubMesh( Auto_group_for_Sub_mesh_5, 'Sub-mesh_5' )
Sub_mesh_6 = Mesh_5.GetSubMesh( Auto_group_for_Sub_mesh_6, 'Sub-mesh_6' )
Sub_mesh_7 = Mesh_5.GetSubMesh( Face_11, 'Sub-mesh_7' )

status = Mesh_5.AddHypothesis(Local_Length_2)
status = Mesh_5.AddHypothesis(Regular_1D)
status = Mesh_5.AddHypothesis(NETGEN_2D)
status = Mesh_5.AddHypothesis(Prism_3D)
status = Mesh_5.AddHypothesis(Regular_1D,Auto_group_for_Sub_mesh_5)
status = Mesh_5.AddHypothesis(Number_of_Segments_2,Auto_group_for_Sub_mesh_5)
status = Mesh_5.AddHypothesis(Regular_1D,Auto_group_for_Sub_mesh_6)
status = Mesh_5.AddHypothesis(Local_Length_1,Auto_group_for_Sub_mesh_6)
status = Mesh_5.AddHypothesis(NETGEN_2D_Parameters_3)
status = Mesh_5.AddHypothesis(Regular_1D,Face_11)
status = Mesh_5.AddHypothesis(Local_Length_2,Face_11)
status = Mesh_5.AddHypothesis(NETGEN_2D,Face_11)
status = Mesh_5.AddHypothesis(NETGEN_2D_Parameters_3,Face_11)
status = Mesh_5.AddHypothesis(Viscous_Layers_2D_1,Face_11)
status = Mesh_5.AddHypothesis(Viscous_Layers_2D_2,Face_11)
isDone = Mesh_5.SetMeshOrder( [ [ Sub_mesh_6, Sub_mesh_7 ] ])


def setHeaterMesh(mesh, edge, subedge, face, subface):
    #
    Viscous_Layers_2D_3.SetEdges( [ ], 1 )

    #
    status = mesh.AddHypothesis(Regular_1D)
    status = mesh.AddHypothesis(Regular_1D, edge)
    status = mesh.AddHypothesis(Regular_1D, face)
    status = mesh.AddHypothesis(NETGEN_2D)
    status = mesh.AddHypothesis(NETGEN_2D, face)
    status = mesh.AddHypothesis(Prism_3D)
    #
    status = mesh.AddHypothesis(Local_Length_1)
    status = mesh.AddHypothesis(Number_of_Segments_2, edge)
    status = mesh.AddHypothesis(Local_Length_1, face)

    status = mesh.AddHypothesis(NETGEN_2D_Parameters_1)
    status = mesh.AddHypothesis(NETGEN_2D_Parameters_1, face)

    status = mesh.AddHypothesis(Viscous_Layers_2D_3, face)
    #
    mesh.SetMeshOrder( [ [ subedge, subface ] ])

#
setHeaterMesh(Mesh_1, Edge_26, Sub_mesh_1, Face_18, Sub_mesh_11)
setHeaterMesh(Mesh_2, Edge_29, Sub_mesh_2, Face_21, Sub_mesh_12)
setHeaterMesh(Mesh_3, Edge_32, Sub_mesh_3, Face_24, Sub_mesh_13)
setHeaterMesh(Mesh_4, Edge_35, Sub_mesh_4, Face_27, Sub_mesh_14)


aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_16,SMESH.FT_Undefined,SMESH.FT_LogicalOR)
aCriteria.append(aCriterion)
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_18)
aCriteria.append(aCriterion)
aFilter_7 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_7.SetMesh(Mesh_1.GetMesh())
frontAndBack_1 = Mesh_1.GroupOnFilter( SMESH.FACE, 'frontAndBack', aFilter_7 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_17)
aCriteria.append(aCriterion)
aFilter_8 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_8.SetMesh(Mesh_1.GetMesh())
InterfaceHeater1 = Mesh_1.GroupOnFilter( SMESH.FACE, 'InterfaceHeater1', aFilter_8 )


aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_19,SMESH.FT_Undefined,SMESH.FT_LogicalOR)
aCriteria.append(aCriterion)
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_21)
aCriteria.append(aCriterion)
aFilter_9 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_9.SetMesh(Mesh_2.GetMesh())
frontAndBack_2 = Mesh_2.GroupOnFilter( SMESH.FACE, 'frontAndBack', aFilter_9 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_20)
aCriteria.append(aCriterion)
aFilter_10 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_10.SetMesh(Mesh_2.GetMesh())
InterfaceHeater2 = Mesh_2.GroupOnFilter( SMESH.FACE, 'InterfaceHeater2', aFilter_10 )


aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_22,SMESH.FT_Undefined,SMESH.FT_LogicalOR)
aCriteria.append(aCriterion)
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_24)
aCriteria.append(aCriterion)
aFilter_11 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_11.SetMesh(Mesh_3.GetMesh())
frontAndBack_3 = Mesh_3.GroupOnFilter( SMESH.FACE, 'frontAndBack', aFilter_11 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_23)
aCriteria.append(aCriterion)
aFilter_12 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_12.SetMesh(Mesh_3.GetMesh())
InterfaceHeater3 = Mesh_3.GroupOnFilter( SMESH.FACE, 'InterfaceHeater3', aFilter_12 )


aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_25,SMESH.FT_Undefined,SMESH.FT_LogicalOR)
aCriteria.append(aCriterion)
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_27)
aCriteria.append(aCriterion)
aFilter_13 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_13.SetMesh(Mesh_4.GetMesh())
frontAndBack_4 = Mesh_4.GroupOnFilter( SMESH.FACE, 'frontAndBack', aFilter_13 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_26)
aCriteria.append(aCriterion)
aFilter_14 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_14.SetMesh(Mesh_4.GetMesh())
InterfaceHeater4 = Mesh_4.GroupOnFilter( SMESH.FACE, 'InterfaceHeater4', aFilter_14 )


aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_10,SMESH.FT_Undefined,SMESH.FT_LogicalOR)
aCriteria.append(aCriterion)
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_11)
aCriteria.append(aCriterion)
aFilter_18 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_18.SetMesh(Mesh_5.GetMesh())
frontAndBack = Mesh_5.GroupOnFilter( SMESH.FACE, 'frontAndBack', aFilter_18 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_9)
aCriteria.append(aCriterion)
aFilter_19 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_19.SetMesh(Mesh_5.GetMesh())
bottom = Mesh_5.GroupOnFilter( SMESH.FACE, 'bottom', aFilter_19 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_12)
aCriteria.append(aCriterion)
aFilter_20 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_20.SetMesh(Mesh_5.GetMesh())
top = Mesh_5.GroupOnFilter( SMESH.FACE, 'top', aFilter_20 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_15)
aCriteria.append(aCriterion)
aFilter_21 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_21.SetMesh(Mesh_5.GetMesh())
right = Mesh_5.GroupOnFilter( SMESH.FACE, 'right', aFilter_21 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_6)
aCriteria.append(aCriterion)
aFilter_22 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_22.SetMesh(Mesh_5.GetMesh())
left = Mesh_5.GroupOnFilter( SMESH.FACE, 'left', aFilter_22 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_14)
aCriteria.append(aCriterion)
aFilter_26 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_26.SetMesh(Mesh_5.GetMesh())
heater1 = Mesh_5.GroupOnFilter( SMESH.FACE, 'heater1', aFilter_26 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_13)
aCriteria.append(aCriterion)
aFilter_25 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_25.SetMesh(Mesh_5.GetMesh())
heater2 = Mesh_5.GroupOnFilter( SMESH.FACE, 'heater2', aFilter_25 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_7)
aCriteria.append(aCriterion)
aFilter_23 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_23.SetMesh(Mesh_5.GetMesh())
heater3 = Mesh_5.GroupOnFilter( SMESH.FACE, 'heater3', aFilter_23 )

aCriteria = []
aCriterion = smesh.GetCriterion(SMESH.FACE,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Face_8)
aCriteria.append(aCriterion)
aFilter_24 = smesh.GetFilterFromCriteria(aCriteria)
aFilter_24.SetMesh(Mesh_5.GetMesh())
heater4 = Mesh_5.GroupOnFilter( SMESH.FACE, 'heater4', aFilter_24 )


## Set names of Mesh objects
smesh.SetName(Regular_1D, 'Regular_1D')
smesh.SetName(Prism_3D, 'Prism_3D')
smesh.SetName(NETGEN_2D, 'NETGEN 2D')

smesh.SetName(Number_of_Segments_1, 'Number of Segments_1')
smesh.SetName(Number_of_Segments_2, 'Number of Segments_2')

smesh.SetName(Local_Length_1, 'Local Length_1')
smesh.SetName(Local_Length_2, 'Local Length_2')

smesh.SetName(Viscous_Layers_2D_1, 'Viscous Layers 2D_1')
smesh.SetName(Viscous_Layers_2D_2, 'Viscous Layers 2D_2')
smesh.SetName(Viscous_Layers_2D_3, 'Viscous Layers 2D_3')

smesh.SetName(NETGEN_2D_Parameters_1, 'NETGEN 2D Parameters_1')
smesh.SetName(NETGEN_2D_Parameters_3, 'NETGEN 2D Parameters_3')

smesh.SetName(Sub_mesh_7, 'Sub-mesh_7')
smesh.SetName(Sub_mesh_6, 'Sub-mesh_6')
smesh.SetName(Sub_mesh_5, 'Sub-mesh_5')
smesh.SetName(Sub_mesh_14, 'Sub-mesh_14')
smesh.SetName(Sub_mesh_13, 'Sub-mesh_13')
smesh.SetName(Sub_mesh_12, 'Sub-mesh_12')
smesh.SetName(Sub_mesh_11, 'Sub-mesh_11')
smesh.SetName(Sub_mesh_4, 'Sub-mesh_4')
smesh.SetName(Sub_mesh_3, 'Sub-mesh_3')
smesh.SetName(Sub_mesh_2, 'Sub-mesh_2')
smesh.SetName(Sub_mesh_1, 'Sub-mesh_1')
smesh.SetName(Mesh_1.GetMesh(), 'Mesh_1')
smesh.SetName(Mesh_2.GetMesh(), 'Mesh_2')
smesh.SetName(Mesh_3.GetMesh(), 'Mesh_3')
smesh.SetName(Mesh_4.GetMesh(), 'Mesh_4')
smesh.SetName(Mesh_5.GetMesh(), 'Mesh_5')

smesh.SetName(top, 'top')
smesh.SetName(bottom, 'bottom')
smesh.SetName(right, 'right')
smesh.SetName(left, 'left')
smesh.SetName(heater1, 'heater1')
smesh.SetName(heater2, 'heater2')
smesh.SetName(heater3, 'heater3')
smesh.SetName(heater4, 'heater4')
smesh.SetName(frontAndBack, 'frontAndBack')


if salome.sg.hasDesktop():
    #
    salome.sg.updateObjBrowser()
    #
    isDone = Mesh_1.Compute()
    isDone = Mesh_2.Compute()
    isDone = Mesh_3.Compute()
    isDone = Mesh_4.Compute()
    isDone = Mesh_5.Compute()

else:
    #
    isDone = Mesh_1.Compute()
    isDone = Mesh_2.Compute()
    isDone = Mesh_3.Compute()
    isDone = Mesh_4.Compute()
    isDone = Mesh_5.Compute()
#
