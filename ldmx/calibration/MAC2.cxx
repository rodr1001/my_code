#include "Framework/EventProcessor.h"
#include "Ecal/Event/EcalHit.h"
#include <iostream>
#include <string>
#include "Ecal/EcalRecProducer.h"

using namespace std;
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalGeometry.h"
#include <fstream>


class MAC2 : public framework::Analyzer {
 public:
  MAC2(const std::string& name, framework::Process& p)
    : framework::Analyzer(name, p) {}
  ~MAC2() = default;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;
};

std::string histname_cell_suffix(ldmx::EcalID id) {
  auto [u, v] = id.getCellUV();
  return (
    "_l"+std::to_string(id.layer())+"_m"+std::to_string(id.module())+"_u"+std::to_string(u)+"_v"+std::to_string(v)
  );
}

//void EcalRecProducer::produce(framework::Event& event) {
  // Get the Ecaldition<ldmx::EcalGeometry>           ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);
void MAC2::onProcessStart() {
  getHistoDirectory();
    // this is where we will define the histograms we want to fill
    for (unsigned int layer{0}; layer < 34; layer++) {
        for (unsigned int cell{0}; cell < 432; cell++) {
            ldmx::EcalID id{layer, 0, cell};
            auto [u, v] = id.getCellUV();
        histograms_.create("cell_amplitude"+histname_cell_suffix(id),
        "Hit Amplitude / MeV", 100, 0.0, 3.0
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
       histograms_.fill("cell_amplitude"+histname_cell_suffix(id), hit.getAmplitude());
     }
     // std::cout << [u,v];
  }
}
DECLARE_ANALYZER(MAC2);

