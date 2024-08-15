#include <iostream>
#include <string>

#include "Ecal/EcalRecProducer.h"
#include "Ecal/Event/EcalHit.h"
#include "Framework/EventProcessor.h"

using namespace std;
#include <cmath>
#include <fstream>

#include "DetDescr/EcalGeometry.h"
#include "DetDescr/EcalID.h"
class CalibGeom;

// cell thickness in millimeters (0.3 mm), gonna leave this as just 1, these
// numbers are arbitrary.
//
class MAC2 : public framework::Analyzer {
 public:
  MAC2(const std::string& name, framework::Process& p)
      : framework::Analyzer(name, p) {}
  ~MAC2() = default;
  void configure(framework::config::Parameters& ps) final;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;

 private:
  double seed_thresh_;
  std::vector<ldmx::EcalID> find_seeds(
      const std::map<ldmx::EcalID, const ldmx::EcalHit&>& hit_by_id,
      const CalibGeom& cg);
};

void MAC2::configure(framework::config::Parameters& ps) {
  // default seed threshold at half a nominal MIP
  seed_thresh_ = ps.getParameter<double>("seed_thresh", 0.13 / 2);
}

// function for the cell suffix
std::string histname_cell_suffix(ldmx::EcalID id) {
  auto [u, v] = id.getCellUV();
  return ("_l" + std::to_string(id.layer()) + "_m" +
          std::to_string(id.module()) + "_u" + std::to_string(u) + "_v" +
          std::to_string(v));
}

class CalibGeom {
 public:
  CalibGeom(const ldmx::EcalGeometry& geometry, double beam_x, double beam_y,
            double beam_z)
      : geometry_{geometry}, beamx_{beam_x}, beamy_{beam_y}, beamz_{beam_z} {}

  // a lil function to calculate the path length //input the x,y and z
  // coordinates, can i create a function that will take the id?
  double path_length(ldmx::EcalID id) const {
    auto [x, y, z] = geometry_.getPosition(id);
    double mag =
        pow((x - beamx_), 2) + pow((y - beamy_), 2) + pow(z - beamz_, 2);
    auto distance = sqrt(mag);
    auto cosangle = (z - beamz_) / distance;
    auto length = thickness_ / cosangle;
    return length;
  }

  double normalized_ampl(const ldmx::EcalHit& hit) const {
    return hit.getAmplitude() / path_length(hit.getID());
  }

  const ldmx::EcalGeometry& ecg() const { return geometry_; }

  std::tuple<double, double> project_to_layer(const ldmx::EcalID& thru,
                                              int ilayer) const {
    double zf = geometry_.getZPosition(ilayer);
    auto [x, y, z] = geometry_.getPosition(thru);
    double xf = (x - beamx_) * (zf - beamz_) / (z - beamz_) + beamx_;
    double yf = (y - beamy_) * (zf - beamz_) / (z - beamz_) + beamy_;
    return std::tuple<double, double>(xf, yf);
  }

 private:
  static constexpr double thickness_ = 1.0;

  const ldmx::EcalGeometry& geometry_;
  double beamx_, beamy_, beamz_;
};

void MAC2::onProcessStart() {
  getHistoDirectory();
  // this is where we will define the histograms we want to fill
  for (unsigned int layer{0}; layer < 34; layer++) {
    for (unsigned int cell{0}; cell < 432; cell++) {
      ldmx::EcalID id{layer, 0, cell};
      auto [u, v] = id.getCellUV();
      histograms_.create("cell_amplitude" + histname_cell_suffix(id),
                         "Hit Amplitude per Path Length / (MeV/mm)", 100, 0.0,
                         3.0);
      histograms_.get("cell_amplitude" + histname_cell_suffix(id))
          ->SetTitle(("Hit Amplitude in Central Module, Layer " +
                      std::to_string(layer) + " Cell " + std::to_string(cell) +
                      " (" + std::to_string(u) + "," + std::to_string(v) + ")")
                         .c_str());
    }
  }
}

std::vector<ldmx::EcalID> MAC2::find_seeds(
    const std::map<ldmx::EcalID, const ldmx::EcalHit&>& hit_by_id,
    const CalibGeom& geometry) {
  std::vector<ldmx::EcalID> seed_list = {};

  for (const auto& [id, hit] : hit_by_id) {
    // convert MeV of hit energy to GeV for histogram
    if (id.module() != 0) continue; // require core module
    if (id.layer() !=0) continue; // require layer 0 only
        
    double norm_seed = geometry.normalized_ampl(hit);
    std::cout <<"   " << id <<  "Normalized Amplitude of hit " << norm_seed << std::endl;
    if (norm_seed < seed_thresh_) continue;  // consider only hits above the seeding threshold

    // search cells in NN and NNN around the possible seed for other hits
    // above ??? threshold
    auto NList = geometry.ecg().getNN(id);
    auto NNearList = geometry.ecg().getNNN(id);
    NList.insert(NList.end(), NNearList.begin(), NNearList.end());
    int count = 1;
    for (const auto& cellid : NList) {
      auto ihit = hit_by_id.find(cellid);
      if (ihit == hit_by_id.end()) continue;
      double norm_near = geometry.normalized_ampl(ihit->second);
      if (norm_near > norm_seed) {
        count = count + 1000;  // Spoil this as a seed
        std::cout << "      Neighbor has bigger ampl " << norm_near << std::endl;
        break;
      } else {
        if (norm_near > seed_thresh_) {
          count = count + 1;
          //std::cout << "        Neighbor has amplitude greater than seed threshold " << norm_near << endl;
        }
      }
    }  // close the for loop through neighbours
    if (count ==1) {
      std::cout << "      No Neighbours have amplitude above threshold" << endl;
    }
    if (count > 2 & count < 999){
      std::cout << "   Too many Neighbours have normalized amplitude greater than the threshold" << endl;
    }          
    if (count > 2) continue; // require that we have at most one other energetic hit in same layer nearby
    // now we search down the path to eliminate cases where there is a shower
    // considering some options for algorithm (veto if):
    // (1) number of hits with Enorm>~0.5MIP > [2-4]
    // (2) total energy of hits in 19 > [~3 MIP]
    std::cout << "Accepted as Possible seed. " << std::endl;
    for (int ilayer = 1; ilayer < 5; ilayer++) {
      auto [xl, yl] = geometry.project_to_layer(id, ilayer);
      ldmx::EcalID idl = geometry.ecg().getID(xl, yl, ilayer);  // ID in next layer
      std::vector<ldmx::EcalID> ilNList = geometry.ecg().getNN(idl);
      auto ilNNearList = geometry.ecg().getNNN(idl);
      ilNList.insert(ilNList.end(), ilNNearList.begin(), ilNNearList.end());
      ilNList.push_back(idl);
      int ilcount = 0;
      for (const auto& ncellid : ilNList) {
        auto ilhit = hit_by_id.find(ncellid);
        if (ilhit == hit_by_id.end()) continue;
        double ilnorm_near = geometry.normalized_ampl(ilhit->second);
        if (ilnorm_near > seed_thresh_) {
          ilcount = ilcount + 1;
        }
      }
              
      if (ilcount > 3){
        std::cout << "   Too many cells normalized amplitude greater than the threshold in layer " << ilayer << endl;
        count=1000; // spoil the count to fail
        break; // no need to check other layers...
      }           
    } // loop over layers
    if (count<3) {   // ok, we admit this is a seed....
      seed_list.push_back(id);
      std::cout << "Accepted " << id << " as a seed.  Hurrah." << endl;
    }
  } // end of for loop over all hits        
  return seed_list;
}  // end of seed finding

  void MAC2::analyze(const framework::Event& event) {
    static bool first_event{true};
    const auto& geometry{getCondition<ldmx::EcalGeometry>(
        ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME)};
    if (first_event) {
      first_event = false;

      std::ofstream file("lookuptable.csv");
      cout << "writing lookuptable" << endl;
      file << "CellID,Layer,U,V,X,Y,Z" << endl;
      for (unsigned int layer{0}; layer < 34; layer++) {
        for (unsigned int cell{0}; cell < 432; cell++) {
          ldmx::EcalID id{layer, 0, cell};
          auto [u, v] = id.getCellUV();
          auto [x, y, z] =
              geometry.getPosition(id);  // this should get the position?
          // write id, layer, u, v, x, y, z
          file << id << ",";
          file << layer << ",";
          file << u << ",";
          file << v << ",";
          file << x << ",";
          file << y << ",";
          file << z << endl;
        }
      }
      file.close();
    }
    const auto& ecal_rec_hits{
        event.getCollection<ldmx::EcalHit>("EcalRecHits")};

    // sort the hits by their ID
    std::map<ldmx::EcalID, const ldmx::EcalHit&> hit_by_id;
    for (const auto& hit : ecal_rec_hits) {
      // could implement check on if hit is isolated here
      // then the only hits within the map are hits that happended in the event
      // AND are isolated
      hit_by_id.insert(
          std::pair<ldmx::EcalID, const ldmx::EcalHit&>(hit.getID(), hit));
    }

    double beamx = 0;  // assume for now
    double beamy = 0;  // assume for now
    double beamz = 0;  // assume for now
    CalibGeom cg(geometry, beamx, beamy, beamz);
    std::cout << "New Event" << std::endl;
    std::vector<ldmx::EcalID> seed_ids = find_seeds(hit_by_id, cg);

  }  // end of void analyse
  DECLARE_ANALYZER(MAC2);
