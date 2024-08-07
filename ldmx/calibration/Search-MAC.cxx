#include "Framework/EventProcessor.h"
#include "Ecal/Event/EcalHit.h"
#include <iostream>
#include <string>
#include "Ecal/EcalRecProducer.h"

using namespace std;
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalGeometry.h"
#include <fstream>
#include <cmath>

// Input The BeamSpot Position
int beam_x = 0;
int beam_y = 0;

//cell thickness in millimeters (0.3 mm), gonna leave this as just 1, these numbers are arbitrary.
double thickness = 1 ;

//nominal mip value 
double mip_val = 0.13

//
class MAC2 : public framework::Analyzer {
 public:
  MAC2(const std::string& name, framework::Process& p)
    : framework::Analyzer(name, p) {}
  ~MAC2() = default;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;
};

// function for the cell suffix
std::string histname_cell_suffix(ldmx::EcalID id) {
  auto [u, v] = id.getCellUV();
  return (
    "_l"+std::to_string(id.layer())+"_m"+std::to_string(id.module())+"_u"+std::to_string(u)+"_v"+std::to_string(v)
  );
}

// a lil function to calculate the path length //input the x,y and z coordinates //can i create a function that will take the id?
double path_length(const ldmx::EcalGeometry&geometry, ldmx::EcalID id) {
    auto [x, y, z] = geometry.getPosition(id);

    double mag = pow((x-beam_x),2) + pow((y-beam_y),2) + pow(z,2);    
    auto distance = sqrt(mag);
    auto cos= z/distance;
    auto length = thickness/cos;
    return length;
  }

// path itself, a function that returns the cell id in the next layer, of hit with a given angle of entry

//need to decide different histograms
void MAC2::onProcessStart() {
  getHistoDirectory();
    // this is where we will define the histograms we want to fill
    for (unsigned int layer{0}; layer < 34; layer++) {
        for (unsigned int cell{0}; cell < 432; cell++) {
            ldmx::EcalID id{layer, 0, cell};
            auto [u, v] = id.getCellUV();
        histograms_.create("cell_amplitude"+histname_cell_suffix(id),
        "Hit Amplitude per Path Length / (MeV/mm)", 100, 0.0, 3.0
        );
        histograms_.get("cell_amplitude"+histname_cell_suffix(id))->SetTitle(
            (
            "Hit Amplitude in Central Module, Layer "+std::to_string(layer)
            +" Cell "+std::to_string(cell)
            +" ("+std::to_string(u)+","+std::to_string(v)+")"
        ).c_str()
      );
    }
  }
}

void MAC2::analyze(const framework::Event& event) {
  static bool first_event{true};
  if (first_event) {
    first_event = false;
    const auto& geometry{getCondition<ldmx::EcalGeometry>(ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME)};
    
    std::ofstream file("lookuptable.csv");
    cout << "writing lookuptable" << endl; 
    file << "CellID,Layer,U,V,X,Y,Z" << endl;
    for (unsigned int layer{0}; layer < 34; layer++) {
      for (unsigned int cell{0}; cell < 432; cell++) {
        ldmx::EcalID id{layer, 0, cell};
        auto [u, v] = id.getCellUV();
        auto [x, y, z] = geometry.getPosition(id);// this should get the position?
        // write id, layer, u, v, x, y, z
        file << id <<",";
        file << layer <<",";
        file << u <<","; file << v <<",";
        file << x <<",";file << y <<",";file << z << endl;
      
}
    }
    file.close();
  }
  const auto& ecal_rec_hits{event.getCollection<ldmx::EcalHit>("EcalRecHits")};
  // std::vector<ldmx::EcalHit>
  for (const auto& hit : ecal_rec_hits) {
     // convert MeV of hit energy to GeV for histogram
     ldmx::EcalID id{static_cast<unsigned int>(hit.getID())};
     if (id.module() == 0) {
       // only hits in core module
        // start with id.layer()== 0 
        // if epl > (0.5*mip)
            // get Nearest and Next Nearest cells id (list)
             // for i in list, get epls, j =1 
             // if epl > 0.5, j =+ 1
             // if j < 2         
 const auto& geometry{getCondition<ldmx::EcalGeometry>(ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME)};       
        auto epl = hit.getAmplitude()/path_length(geometry,id); // energy per unit length
        // if any fail this, end loop. 
        // each of the 6 cells must have an epl below the threshold
        // passing this check, then we accept this hit
     histograms_.fill("cell_amplitude"+histname_cell_suffix(id), epl);
     }
     // std::cout << [u,v];
  }
}
DECLARE_ANALYZER(MAC2);

