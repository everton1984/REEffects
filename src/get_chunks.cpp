#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <vector>

//GIS related includes
#include "ogrsf_frmts.h"

//CGAL related includes
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Triangular_expansion_visibility_2.h>

#include <CGAL/intersections.h>

#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>

#include <boost/algorithm/string.hpp>

//Default configuration path
#define CONFIG_FILE "./cfg/config"

using namespace::std;

typedef CGAL::Exact_predicates_exact_constructions_kernel                       Kernel;
typedef CGAL::Arr_segment_traits_2<Kernel>                                      Traits_2;
typedef CGAL::Arrangement_2<Traits_2>                                           Arrangement_2;

typedef Arrangement_2::Face_handle                                              Face_handle;
typedef Arrangement_2::Face_const_handle                                        Face_const_handle;
typedef Arrangement_2::Halfedge_const_handle                                    Halfedge_const_handle;

typedef Arrangement_2::Edge_const_iterator                                      Edge_const_iterator;
typedef Arrangement_2::Vertex_const_iterator                                    Vertex_const_iterator;

typedef CGAL::Triangular_expansion_visibility_2<Arrangement_2>                  TEV;

typedef CGAL::Arr_point_location_result<Arrangement_2>                          Point_location_result;

void read_config(string fName);
void read_gis_file(string fname, string plotFname);
void create_environment(Arrangement_2 *arr);
void construct_bounding_polygon(double maxDist, Kernel::Point_2 center, vector<Kernel::Segment_2> *polygon);
void dump_arrangement(Arrangement_2 *arr, string plotFname);

//Each element of this vector is a vector of line segments read from the GIS file.
vector<vector <Kernel::Segment_2>> roads;
//Points read from GIS file.
vector<Kernel::Point_2> points;

//Dictionary with configuration options
unordered_map<string,string> config;

int main(int argc, char *argv[]){
    //Size of the polygon created around each point
    double rmax = 1000;
    //Database containing roads
    Arrangement_2 env;

    if( argc < 2 ){
        cout << "Using " << CONFIG_FILE << " as the configuration file." << endl;
        read_config(CONFIG_FILE);
    } else {
        cout << "Using " << argv[1] << " as the configuration file." << endl;
        read_config(argv[1]);
    }

    rmax = stod(config["rmax"]);

    GDALAllRegister();

    cout << "Reading gis files..." << endl;

    read_gis_file(config["input_roads_file"], config["output_dir"] + "/roads_" + config["output_prefix"] + "_all");
    read_gis_file(config["input_points_file"], config["output_dir"] + "/points_" + config["output_prefix"] + "_all");

    cout << "Number of roads read: " << roads.size() << endl;
    cout << "Number of points read: " << points.size() << endl;

    cout << "Generating roads database..." << endl;
    create_environment(&env);

    int i = 0;
    for(vector<Kernel::Point_2>::iterator it = points.begin(); it != points.end(); ++it ){    
        cout << "Calculating neighborhood of point " << i << endl;

        //Creating bounding polygon for each point, we need to start with a clean environment because
        //adding and removing the polygon each time is not only clumsy but slow.
        Arrangement_2 boundArr = env;
        vector<Kernel::Segment_2> boundingPoly;
        construct_bounding_polygon(rmax,*it,&boundingPoly);
        CGAL::insert(boundArr,boundingPoly.begin(), boundingPoly.end());

        //Get the face that this points belongs.
        Face_const_handle *face;
        CGAL::Arr_naive_point_location<Arrangement_2> pl(boundArr);
        Point_location_result::Type obj = pl.locate(*it);
        face = boost::get<Face_const_handle> (&obj);

        Arrangement_2 output_arr;
        TEV tev(boundArr);

        cout << "\tcomputing visibility..." << endl;

        tev.compute_visibility(*it,*face, output_arr);

        dump_arrangement(&output_arr,config["output_dir"] + "/chunks_raw_" + config["output_prefix"] +  "_" + to_string(i));

        //Once everything is calculated the visibility creates some extra edges. Since
        //we just want real road segments we have to calculate the intersection. 
        Arrangement_2 res;
        vector<Kernel::Segment_2> bounding_roads;
        for( Edge_const_iterator envit = env.edges_begin(); envit != env.edges_end(); ++envit){
            Kernel::Segment_2 env_edge = envit->curve();
            bool found = false;

            for( Edge_const_iterator eit = output_arr.edges_begin(); eit != output_arr.edges_end() && !found; ++eit ){
                Kernel::Segment_2 out_edge = eit->curve();

                CGAL::cpp11::result_of<Kernel::Intersect_2(Kernel::Segment_2,Kernel::Segment_2)>::type 
                    inter = CGAL::intersection(env_edge,out_edge);
                if(inter) {
                    if(const Kernel::Segment_2 *inter_seg = boost::get<Kernel::Segment_2>(&*inter)){
                        bounding_roads.push_back(*inter_seg);
                        found = true;
                    }
                }
            }
        }
        CGAL::insert(res, bounding_roads.begin(), bounding_roads.end());
       
        //Writing everything to output files.
        ofstream pf;
        pf.open(config["output_dir"] + "/points_" + config["output_prefix"] +  "_" + to_string(i));
        pf << setprecision(20);

        if( !pf.is_open() ){
            cout << "Failed opening " << config["output_dir"] << "/points_" << config["output_prefix"] << "_" + to_string(i) << endl;
            exit(1);
        }
        pf << (*it).x() << " " << (*it).y() << endl;
        pf.close();

        dump_arrangement(&res,config["output_dir"] + "/chunks_" + config["output_prefix"]  +  "_" + to_string(i));

        cout << "\tnumber of roads intersecting point: " << res.number_of_edges() << endl;
        i++;
    }
    
    cout << "Finished calculating chunks." << endl;

    return 0;
}

void read_config(string fName){
    ifstream configFile;
    configFile.open(fName.c_str());

    if( !configFile.is_open() ){
        cout << "Failed to open " << fName << " file." << endl;
        exit(1);
    }

    while( !configFile.eof() ){
        string key,value;
        char sep;
        
        configFile >> key >> sep >> value;
        boost::to_lower(key);
        boost::trim(key);
        boost::trim(value);

        if( key.size() > 0 && value.size() > 0){
            config[key] = value;
        }
    }

    configFile.close();
}

void construct_bounding_polygon(double maxDist, Kernel::Point_2 center, vector<Kernel::Segment_2> *polygon){
    Kernel::Point_2 p1(center.x()-maxDist,center.y()-maxDist),
                    p2(center.x()+maxDist,center.y()-maxDist),
                    p3(center.x()-maxDist,center.y()+maxDist),
                    p4(center.x()+maxDist,center.y()+maxDist);
    
    polygon->push_back(Kernel::Segment_2(p1,p2));
    polygon->push_back(Kernel::Segment_2(p2,p4));
    polygon->push_back(Kernel::Segment_2(p4,p3));
    polygon->push_back(Kernel::Segment_2(p3,p1));
}

void dump_arrangement(Arrangement_2 *arr, string plotFname) {
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

void read_gis_file(string fname, string plotFname){
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
        cout << "Failed to open " << fname.c_str() << " file." << endl;
        exit(1);
    }

    for( int id_layer = 0; id_layer < poDS->GetLayerCount(); id_layer++){
        OGRLayer *layer = poDS->GetLayer(id_layer);
        if( layer != NULL ){
            layer->ResetReading();            
            OGRFeature *feat = NULL;
            while( (feat = layer->GetNextFeature()) != NULL ){
                OGRGeometry *geom = feat->GetGeometryRef();
                if( geom ){
                    if( wkbFlatten(geom->getGeometryType()) == wkbPoint ){
                        OGRPoint *p = (OGRPoint *) geom;
                        Kernel::Point_2 point(p->getX(),p->getY());

                        points.push_back(point);
                        
                        plotFile << p->getX() << " " << p->getY() << endl;
                    } else if( wkbFlatten(geom->getGeometryType()) == wkbLineString ){
                        OGRLineString *l = (OGRLineString *) geom;
                        OGRPoint pStart = OGRPoint();
                        OGRPoint pEnd = OGRPoint();
                        vector<Kernel::Segment_2> thisRoad;

                        l->getPoint(0,&pStart);
                        plotFile << pStart.getX() << " " << pStart.getY() << endl;
                        for(int i = 1; i < l->getNumPoints(); i++){
                            l->getPoint(i,&pEnd);
                            
                            plotFile << pEnd.getX() << " " << pEnd.getY() << endl;

                            Kernel::Point_2 pointStart(pStart.getX(),pStart.getY());
                            Kernel::Point_2 pointEnd(pEnd.getX(),pEnd.getY());
                            
                            thisRoad.push_back(Kernel::Segment_2(pointStart,pointEnd));

                            pStart = pEnd;
                        }
                        roads.push_back(thisRoad);
                        plotFile << endl;
                    } else if( wkbFlatten(geom->getGeometryType()) == wkbMultiLineString ){
                        OGRMultiLineString *ml = (OGRMultiLineString *)geom;
                        for(int i = 0; i < ml->getNumGeometries(); i++){
                            /* [DEBUG] Fix me! */
                            if( wkbFlatten(ml->getGeometryRef(i)->getGeometryType()) == wkbLineString ){
                                OGRLineString *l = (OGRLineString *) ml->getGeometryRef(i);
                                OGRPoint pStart = OGRPoint();
                                OGRPoint pEnd = OGRPoint();
                                vector<Kernel::Segment_2> thisRoad;

                                l->getPoint(0,&pStart);
                                plotFile << pStart.getX() << " " << pStart.getY() << endl;
                                for(int i = 1; i < l->getNumPoints(); i++){
                                    l->getPoint(i,&pEnd);
                                    
                                    plotFile << pEnd.getX() << " " << pEnd.getY() << endl;

                                    Kernel::Point_2 pointStart(pStart.getX(),pStart.getY());
                                    Kernel::Point_2 pointEnd(pEnd.getX(),pEnd.getY());
                                    
                                    thisRoad.push_back(Kernel::Segment_2(pointStart,pointEnd));

                                    pStart = pEnd;
                                }
                                roads.push_back(thisRoad);
                                plotFile << endl;                           
                            } else {
                                cout << "[WARNING] Unrecognized geometry: " 
                                    << wkbFlatten(ml->getGeometryRef(i)->getGeometryType()) << endl;
                            }
                        }
                    } else {
                        cout << "[WARNING] Unrecognized geometry: " << wkbFlatten(geom->getGeometryType()) << endl;
                    }
                }

                OGRFeature::DestroyFeature(feat);
            }
        }
    }

    plotFile.close();
    GDALClose(poDS);
}

void create_environment(Arrangement_2 *arr) {
    for( vector< vector <Kernel::Segment_2> >::iterator rit = roads.begin(); rit != roads.end(); ++rit ){
        //We have to use the most generic insert function because we have intersections, sounds weird that maps would
        //have intersecting segments but there's no alternative for now.
        CGAL::insert(*arr, (*rit).begin(), (*rit).end() );
    }
}
