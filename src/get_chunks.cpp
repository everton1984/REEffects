#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "ogrsf_frmts.h"
#include <vector>
#include <CGAL/basic.h>
#include <CGAL/Arr_algebraic_segment_traits_2.h>
//#include <CGAL/Gmpz.h>
//#include <CGAL/CORE_BigInt.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Triangular_expansion_visibility_2.h>
#include <CGAL/Simple_polygon_visibility_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_overlay_2.h>
#include <CGAL/Arr_segment_traits_2.h>
//#include <CGAL/Arr_non_caching_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/number_utils.h>

using namespace::std;

#if CGAL_USE_GMP && CGAL_USE_MPFI
#include <CGAL/Gmpz.h>
typedef CGAL::Gmpz Integer;
#elif CGAL_USE_CORE
#include <CGAL/CORE_BigInt.h>
typedef CORE::BigInt Integer;
#else
#include <CGAL/leda_integer.h>
typedef LEDA::integer Integer;
#endif

//typedef CORE::BigInt                                        Integer;
//typedef CGAL::Gmpz                                          Integer;

typedef CGAL::Exact_predicates_exact_constructions_kernel   Kernel;
typedef CGAL::Arr_segment_traits_2<Kernel>                  Traits_2;
//typedef CGAL::Arr_non_caching_segment_traits_2<Kernel>      Traits_2;
//typedef CGAL::Arr_algebraic_segment_traits_2<Integer>       Arr_traits_2;
//typedef CGAL::Arrangement_2<Arr_traits_2>                   Arr_arrangement_2;
typedef CGAL::Arrangement_2<Traits_2>                       Arrangement_2;
//typedef Arr_traits_2::Curve_2                               Curve_2;
//typedef Arr_traits_2::Polynomial_2                          Polynomial_2;
typedef Arrangement_2::Face_handle                          Face_handle;
typedef Arrangement_2::Edge_const_iterator                  Edge_const_iterator;
//typedef Arrangement_2::Halfedge_const_handle                Halfedge_const_handle;
//typedef Arrangement_2::Ccb_halfedge_circulator              Ccb_halfedge_circulator;

void readGisFile(string fname, string plotFname, bool writeFile);
void constructBoundingPolygon(double maxDist, Kernel::Point_2 center, vector<Kernel::Segment_2> *polygon);
void dumpArrangement(Arrangement_2 *arr, string plotFname);

Arrangement_2 env;
vector<Kernel::Segment_2> roads;
vector<Kernel::Point_2> points;

int main(int argc, char *argv[]){
    double rmax = 10000;
    GDALAllRegister();

    readGisFile("./data/gis/Frag_Macacu_pontos_UTMSAD6923S.shp", "./out/points_all", true);
    readGisFile("./data/gis/estradas_Macacu.shp", "./out/roads_all", true);

    cout << roads.size() << endl;
    cout << points.size() << endl;

    //Arrangement_2 env;
    //CGAL::insert_non_intersecting_curves(env,roads.begin(),roads.end());
    int i = 0;
    for(vector<Kernel::Point_2>::iterator it = points.begin(); it != points.end(); ++it ){    
        /* [DEBUG] This is the case where we have a polygon without holes, just a test */
        /*
        Arr_traits_2 arr_traits;
        Arr_traits_2::Construct_curve_2 construct_curve = arr_traits.construct_curve_2_object();

        Polynomial_2 x = CGAL::shift(Polynomial_2(1),1,0);
        Polynomial_2 y = CGAL::shift(Polynomial_2(1),1,1);

        Arr_arrangement_2 arr(&arr_traits);

        double xp = CGAL::to_double((*it).x());
        double yp = CGAL::to_double((*it).y());
        */
        Arrangement_2 boundArr = env;
        vector<Kernel::Segment_2> boundingPoly;
        constructBoundingPolygon(rmax/2,*it,&boundingPoly);
        //CGAL::insert_non_intersecting_curves(env,boundingPoly.begin(),boundingPoly.end());
        CGAL::insert(boundArr,boundingPoly.begin(), boundingPoly.end());

        /*Polynomial_2 circle = CGAL::ipower(x,2) + 2*x*xp + xp*xp + CGAL::ipower(y,2) + 2*y*yp + yp*yp + rmax;
        Curve_2 circle_curve = construct_curve(circle);
        CGAL::insert(arr,circle_curve);
        CGAL::insert(env,circle_curve);
        */
        Arrangement_2::Face_const_handle *face;
        CGAL::Arr_naive_point_location<Arrangement_2> pl(boundArr);
        CGAL::Arr_point_location_result<Arrangement_2>::Type obj = pl.locate(*it);
        face = boost::get<Arrangement_2::Face_const_handle> (&obj);

        /*typedef CGAL::Simple_polygon_visibility_2<Arrangement_2,CGAL::Tag_true> RSPV;
        Arrangement_2 regular_output;
        RSPV regular_visibility(boundArr);
        
        dumpArrangement(&boundArr,"roads.dat");

        regular_visibility.compute_visibility(*it, *face, regular_output);

        cout << regular_output.number_of_edges() << endl;*/

        typedef CGAL::Triangular_expansion_visibility_2<Arrangement_2> TEV;
        Arrangement_2 output_arr;
        TEV tev(boundArr);
        tev.compute_visibility(*it,*face, output_arr);

        Arrangement_2 res;
        vector<Kernel::Segment_2> bounding_roads;
        for( Edge_const_iterator eit = output_arr.edges_begin(); eit != output_arr.edges_end(); ++eit){
            bool found = false;
            for( Edge_const_iterator envit = env.edges_begin(); envit != env.edges_end() && !found; ++envit ){
                if( envit->source()->point().x() == eit->source()->point().x() &&
                        envit->source()->point().y() == eit->source()->point().y() &&
                        envit->target()->point().x() == eit->target()->point().x() &&
                        envit->target()->point().y() == eit->target()->point().y() ){
                    found = true;
                }
            }
            if( found ) {
                Kernel::Segment_2 seg( eit->source()->point(), eit->target()->point() );
                bounding_roads.push_back(seg);
                //CGAL::insert(res,seg.begin(), seg.end());
            }
        }
        CGAL::insert(res, bounding_roads.begin(), bounding_roads.end());
        //Arrangement_2 overlay;
        //CGAL::overlay(boundArr,output_arr,overlay);

        //dumpArrangement(&output_arr,"roads" + to_string(i) + ".dat");
        //dumpArrangement(&overlay,"roads" + to_string(i) + ".dat");
        ofstream pf;
        pf.open("./out/points_" + to_string(i));
        pf << setprecision(20);

        if( !pf.is_open() ){
            cout << "Failed opening ./out/points_"+to_string(i) << endl;
            exit(1);
        }
        pf << (*it).x() << " " << (*it).y() << endl;
        pf.close();
        dumpArrangement(&res,"./out/chunks_" + to_string(i));

        cout << i << ":" << output_arr.number_of_edges() << endl;
        i++;

        //return 0; 
        /* The more general case starts here  */
        /*typedef CGAL::Triangular_expansion_visibility_2<Arrangement_2> TEV;
        Halfedge_const_handle he = end.halfedges_begin();
        while(he->source()->point() != *it || he->target()->
        */
    }
    return 0;
}

void constructBoundingPolygon(double maxDist, Kernel::Point_2 center, vector<Kernel::Segment_2> *polygon){
    Kernel::Point_2 p1(center.x()-maxDist,center.y()-maxDist),
                    p2(center.x()+maxDist,center.y()-maxDist),
                    p3(center.x()-maxDist,center.y()+maxDist),
                    p4(center.x()+maxDist,center.y()+maxDist);
    
    polygon->push_back(Kernel::Segment_2(p1,p2));
    polygon->push_back(Kernel::Segment_2(p2,p4));
    polygon->push_back(Kernel::Segment_2(p4,p3));
    polygon->push_back(Kernel::Segment_2(p3,p1));
}

void dumpArrangement(Arrangement_2 *arr, string plotFname) {
    ofstream plotFile;
    plotFile.open(plotFname.c_str());
    plotFile << setprecision(20);

    if( !plotFile.is_open() ){
        cout << "Failed to open " << plotFname << " file." << endl;
        exit(1);
    }

    for(Edge_const_iterator eit = arr->edges_begin(); eit != arr->edges_end(); ++eit){
        plotFile << eit->source()->point().x() << " " << eit->source()->point().y() << endl;
        plotFile << eit->target()->point().x() << " " << eit->target()->point().y() << endl;
        plotFile << endl;
    }
    plotFile.close();
}

void readGisFile(string fname, string plotFname, bool writeFile){
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
            while( (feat = layer->GetNextFeature()) != NULL ){ //&& featCount < 100 ){
                featCount++; //[DEBUG] just to get a small number of roads
                OGRGeometry *geom = feat->GetGeometryRef();
                if( geom ){
                    if( wkbFlatten(geom->getGeometryType()) == wkbPoint ){
                        OGRPoint *p = (OGRPoint *) geom;
                        Kernel::Point_2 point(p->getX(),p->getY());

                        points.push_back(point);
                        
                        //cout << p->getX() << " " << p->getY() << endl;
                        if( writeFile ){
                            plotFile << p->getX() << " " << p->getY() << endl;
                        }
                    } else if( wkbFlatten(geom->getGeometryType()) == wkbLineString ){
                        OGRLineString *l = (OGRLineString *) geom;
                        OGRPoint pStart = OGRPoint();
                        OGRPoint pEnd = OGRPoint();
                        vector<Kernel::Segment_2> thisRoad;

                        //if( l->getNumPoints() > 400 ){
                            l->getPoint(0,&pStart);
                            //cout << "Road " << featCount << ":" << l->getNumPoints() << endl; 
                            for(int i = 1; i < l->getNumPoints(); i++){
                                l->getPoint(i,&pEnd);
                                
                                //cout << pStart.getX() << " " << pStart.getY() << endl;
                                if( writeFile ){
                                    plotFile << pStart.getX() << " " << pStart.getY() << endl;
                                }

                                Kernel::Point_2 pointStart(pStart.getX(),pStart.getY());
                                Kernel::Point_2 pointEnd(pEnd.getX(),pEnd.getY());
                                roads.push_back(Kernel::Segment_2(pointStart,pointEnd) );
                                thisRoad.push_back(Kernel::Segment_2(pointStart,pointEnd));

                                pStart = pEnd;
                            }
                            //cout << thisRoad.size() << endl;
                            //CGAL::insert_non_intersecting_curves(env,thisRoad.begin(),thisRoad.end());
                            CGAL::insert(env,thisRoad.begin(),thisRoad.end());
                            if( writeFile ){
                                plotFile << endl;
                            }
                        //}
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
