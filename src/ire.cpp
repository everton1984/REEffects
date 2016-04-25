#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "ogrsf_frmts.h"
#include <vector>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Triangular_expansion_visibility_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>

using namespace::std;

typedef CGAL::Exact_predicates_exact_constructions_kernel   Kernel;
typedef CGAL::Arr_segment_traits_2<Kernel>                  Traits_2;
typedef CGAL::Arrangement_2<Traits_2>                       Arrangement_2;
typedef Arrangement_2::Face_handle                          Face_handle;
typedef Arrangement_2::Edge_const_iterator                  Edge_const_iterator;
typedef Arrangement_2::Halfedge_const_handle                Halfedge_const_handle;
typedef Arrangement_2::Ccb_halfedge_circulator              Ccb_halfedge_circulator;

void readGisFile(string fname, string plotFname);

Arrangement_2 env;
vector<Kernel::Segment_2> roads;
vector<Kernel::Point_2> points;

int main(int argc, char *argv[]){
    GDALAllRegister();

    readGisFile("./data/gis/Frag_Macacu_pontos_UTMSAD6923S.shp", "points.dat");
    readGisFile("./data/gis/estradas_Macacu.shp", "roads.dat");

    cout << roads.size() << endl;
    cout << points.size() << endl;

    //Arrangement_2 env;
    //CGAL::insert_non_intersecting_curves(env,roads.begin(),roads.end());
    for(vector<Kernel::Point_2>::iterator it = points.begin(); it != points.end(); ++it ){    
        /* [DEBUG] This is the case where we have a polygon without holes, just a test */
        Arrangement_2::Face_const_handle *face;
        CGAL::Arr_naive_point_location<Arrangement_2> pl(env);
        CGAL::Arr_point_location_result<Arrangement_2>::Type obj = pl.locate(*it);
        face = boost::get<Arrangement_2::Face_const_handle> (&obj);

        typedef CGAL::Simple_polygon_visibility_2<Arrangement_2,CGAL::Tag_true> RSPV;
        Arrangement_2 regular_output;
        RSPV regular_visibility(env);

        cout << regular_output.number_of_edges() << endl;
        
        /* The more general case starts here  */
        /*typedef CGAL::Triangular_expansion_visibility_2<Arrangement_2> TEV;
        Halfedge_const_handle he = end.halfedges_begin();
        while(he->source()->point() != *it || he->target()->
        */
    }
    return 0;
}

void readGisFile(string fname, string plotFname){
    ofstream plotFile;

    plotFile.open(plotFname.c_str());
    plotFile << setprecision(20);

    if( !plotFile.is_open() ){
        cout << "Failed to open " << plotFname << " file." << endl;
        exit(1);
    }


    GDALDataset *poDS;
    poDS = (GDALDataset *)GDALOpenEx( fname.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if( poDS == NULL ){
        cout << "Open failed\n";
        exit(1);
    }

    //cout << "Layer count: " << poDS->GetLayerCount() << endl;
    for( int id_layer = 0; id_layer < poDS->GetLayerCount(); id_layer++){
        OGRLayer *layer = poDS->GetLayer(id_layer);
        if( layer != NULL ){
            //cout << "\t" << id_layer << ":" << layer->GetFeatureCount() << endl;
            layer->ResetReading();            
            OGRFeature *feat = NULL;
            int featCount = 0;
            while( (feat = layer->GetNextFeature()) != NULL && featCount < 300 ){
                featCount++; //[DEBUG] just to get a small number of roads
                OGRGeometry *geom = feat->GetGeometryRef();
                if( geom ){
                    if( wkbFlatten(geom->getGeometryType()) == wkbPoint ){
                        OGRPoint *p = (OGRPoint *) geom;
                        Kernel::Point_2 point(p->getX(),p->getY());

                        points.push_back(point);
                        
                        //cout << p->getX() << " " << p->getY() << endl;
                        plotFile << p->getX() << " " << p->getY() << endl;
                    } else if( wkbFlatten(geom->getGeometryType()) == wkbLineString ){
                        OGRLineString *l = (OGRLineString *) geom;
                        OGRPoint pStart = OGRPoint();
                        OGRPoint pEnd = OGRPoint();
                        vector<Kernel::Segment_2> thisRoad;

                        l->getPoint(0,&pStart);
                        
                        for(int i = 1; i < l->getNumPoints(); i++){
                            l->getPoint(i,&pEnd);
                            
                            //cout << pStart.getX() << " " << pStart.getY() << endl;
                            plotFile << pStart.getX() << " " << pStart.getY() << endl;
                            
                            Kernel::Point_2 pointStart(pStart.getX(),pStart.getY());
                            Kernel::Point_2 pointEnd(pEnd.getX(),pEnd.getY());
                            roads.push_back(Kernel::Segment_2(pointStart,pointEnd) );
                            thisRoad.push_back(Kernel::Segment_2(pointStart,pointEnd));

                            pStart = pEnd;
                        }
                        
                        CGAL::insert_non_intersecting_curves(env,thisRoad.begin(),thisRoad.end());
                        plotFile << endl;
                        //cout << wkbFlatten(geom->getGeometryType()) << endl;
                        //plotFile << wkbFlatten(geom->getGeometryType()) << endl;
                    }
                }

                OGRFeature::DestroyFeature(feat);
            }
        }
    }

    plotFile.close();
    GDALClose(poDS);
}
