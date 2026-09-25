#include "Framework/EventProcessor.h"
#include "Tracking/Event/Track.h" 
#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/Event/SimParticle.h"
#include "DetDescr/SimSpecialID.h"
#include "Ecal/Event/EcalHit.h"
#include "Recon/Event/CalorimeterHit.h"
#include "Hcal/Event/HcalHit.h"
#include <iostream>
#include <optional>
#include <vector>
#include <random>
#include <set>


template<typename M>
M mag(const std::vector<M>& v) {
  M mag2{0};
  for (const auto& c: v) {
    mag2 += c*c;
  }
  return std::sqrt(mag2);
}
std::tuple<double,double> getImpactPoint(const ldmx::Track::TrackState &state) {
  return std::make_tuple(state.params[0], state.params[1]);
}

std::vector<const ldmx::Track*> sortByMomentum(const std::vector<ldmx::Track>& tracks) {
  std::vector<const ldmx::Track*> sorted_tracks;
  sorted_tracks.reserve(tracks.size());
  for (const auto& track : tracks) { sorted_tracks.emplace_back(&track); }
  std::sort(
      sorted_tracks.begin(),
      sorted_tracks.end(),
      [](const ldmx::Track* lhs, const ldmx::Track* rhs) {
      return mag(lhs->getMomentum()) > mag(rhs->getMomentum());
      }
      );
  return sorted_tracks;
}

class ZoneZeroTrackAnalyzer : public framework::Analyzer {
  int no_leading_electron_count = 0;
  int zone0_count = 0;
  int zone1_count =0;
  int zone2_count = 0;
  int zone3_count = 0;
  int zone4_count = 0;
  int zone5_count = 0;
  int GoodTracks = 0;
  std::set<std::pair<int,int>> IDs;

  public:
  ZoneZeroTrackAnalyzer(const std::string& name, framework::Process& p)
    : framework::Analyzer(name, p) {}
  ~ZoneZeroTrackAnalyzer() = default;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;
  void onProcessEnd() final {
    // std::cout << "ID list (run, event #) = " << IDs << std::endl;
    //std::cout << "hits with no leading electron = " << no_leading_electron_count << std::endl;
    std::cout << "total zone 0 count = " << zone0_count << std::endl;
    // std::cout << "zone 1 count = "<< zone1_count <<std::endl;
    // std::cout << "zone 2 count = "<< zone2_count << std::endl;
    // std::cout << "zone 3 count = "<< zone3_count << std::endl;
    // std::cout << "zone 4 count = "<< zone4_count <<  std::endl;
    // std::cout << "zone 5 count = " << zone5_count <<  std::endl;
    //std::cout << "total energy estimate too big count = " << estimate_too_big_count << std::endl;
    std::cout << "Total Lead Tracks, negatively charged, within beamspot and with 10 hits = " << GoodTracks<< std::endl;
  }
};


void ZoneZeroTrackAnalyzer::onProcessStart () {
  // define the histograms I want to fill
  getHistoDirectory(); 

  histograms_.create("reco_momentum",
      "Track Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("esph_momentum",
      "Hit Momentum at ECal (GeV)", 100, 0, 10);


  histograms_.create("reco_leadtrk_momentum",
      "Lead Track Reco Momentum (GeV)",100,0,10);
  histograms_.create("reco_leadtrk_momentum_all_reqs",
      "Lead Track Reco Momentum (GeV) within Beamspot", 100, 0, 10);
  histograms_.create("leadtrk_reco_vs_esph_momentum",
      "Hit Momentum at ECal (GeV)", 100, 0, 10,
      "Lead Track Reco Momentum (GeV)", 100,0, 10);

  histograms_.create("leadtrk_reco_all_reqs_vs_Energy_Estimate_Beamspot",
      "Energy Estimate within Beamspot (GeV)", 100,0,10,
      "Lead Track Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("leadtrk_reco_all_reqs_vs_ecal_energy",
      "Total Ecal Energy (GeV)", 100, 0,10,
      "Lead Track Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("leadtrk_reco_all_reqs_vs_ecal_energy_PE_cuts",
      "Total Ecal Energy (GeV)", 100, 0,10,
      "Lead Track Reco Momentum (GeV)", 100, 0, 10);


  histograms_.create("leadtrk_reco_all_reqs_vs_ecal_energy_zone_0", 
      "Total Ecal Energy (GeV)", 100, 0,10,
      "Lead Track Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("leadtrk_reco_all_reqs_vs_ecal_energy_zone_else",
      "Total Ecal Energy (GeV)", 100, 0,10,
      "Lead Track Reco Momentum (GeV)", 100, 0, 10);


  histograms_.create("energy_estimate_vs_ecal_energy",
      "Energy Estimate", 100, 0, 10,
      "Total Ecal Energy", 100, 0, 10);
  histograms_.create("ecal_energy", 
      "Total ECal Hit Energy (GeV)", 100, 0, 10);
  histograms_.create("ecal_energy_zone_0",
      "Total ECal Hit Energy - Zone 0 (GeV)", 100, 0, 10);

  histograms_.create("ecal_energy_event_1",
      "Total ECal Hit Energy (GeV) - Fred", 100, 0, 10);
  histograms_.create("ecal_energy_event_2",
      "Total ECal Hit Energy (GeV) - Daphne", 100, 0, 10);
  histograms_.create("ecal_energy_event_3",
      "Total ECal Hit Energy (GeV) - Velma", 100, 0, 10);
  histograms_.create("ecal_energy_event_4",
      "Total ECal Hit Energy (GeV) - Shaggy", 100, 0, 10);
  histograms_.create("ecal_energy_event_5",
      "Total ECal Hit Energy (GeV) - Scooby", 100, 0, 10);
  histograms_.create("ecal_energy_event_6",
      "Total ECal Hit Energy (GeV) - Scrappy", 100, 0, 10);


  histograms_.create("max_pe", "Max PE(HCal)", 100,0,100);

  histograms_.create("max_pe_event_1_Fred", "Max PE(HCal)", 100, 0, 100);
  histograms_.create("max_pe_event_2_Daphne", "Max PE(HCal)", 100, 0, 100);
  histograms_.create("max_pe_event_3_Velma", "Max PE(HCal)", 100, 0, 100);
  histograms_.create("max_pe_event_4_Shaggy", "Max PE(HCal)", 100, 0, 100);
  histograms_.create("max_pe_event_5_Scooby", "Max PE(HCal)", 100, 0, 100);
  histograms_.create("max_pe_event_6_Scrappy", "Max PE(HCal)", 100, 0, 100);

histograms_.create("max_pe_vs_total_ecal_energy_event_1_Fred",
    "Total Ecal Energy (GeV)", 100,0,10,
    "Max PE(HCal)", 100,0,100);
histograms_.create("max_pe_vs_total_ecal_energy_event_2_Daphne",
    "Total Ecal Energy (GeV)", 100,0,10,
    "Max PE(HCal)", 100,0,100);
histograms_.create("max_pe_vs_total_ecal_energy_event_3_Velma",
    "Total Ecal Energy (GeV)", 100,0,10,
    "Max PE(HCal)", 100,0,100);
histograms_.create("max_pe_vs_total_ecal_energy_event_4_Shaggy",
    "Total Ecal Energy (GeV)", 100,0,10,
    "Max PE(HCal)", 100,0,100);
histograms_.create("max_pe_vs_total_ecal_energy_event_5_Scooby",
    "Total Ecal Energy (GeV)", 100,0,10,
    "Max PE(HCal)", 100,0,100);
histograms_.create("max_pe_vs_total_ecal_energy_event_6_Scrappy",
    "Total Ecal Energy (GeV)", 100,0,10,
    "Max PE(HCal)", 100,0,100);
}



void ZoneZeroTrackAnalyzer::analyze(const framework::Event& event) {

  // looking at EcalRecHits
  double total_ecal_energy {0.0};
  const auto& ecal_hits{event.getCollection<ldmx::EcalHit>("EcalRecHits","")};

  for (const auto& hit: ecal_hits){
    total_ecal_energy += (hit.getEnergy()/1000);
  }

  histograms_.fill("ecal_energy", total_ecal_energy);

  //Looking at HcalRecHits
  const auto& hcal_hits{event.getCollection<ldmx::HcalHit>("HcalRecHits","")};
  auto max_pe = 0;
  for (const auto& hit: hcal_hits){
    auto pe = hit.getPE();
    if (pe > max_pe) {
      max_pe = pe;
    }
  }
  histograms_.fill("max_pe", max_pe);
  int run = event.getEventHeader().getRun();
  int event_number = event.getEventNumber();

  //Scrappy first 
  if ((run == 189) && (event_number == 285891)){
    histograms_.fill("max_pe_event_6_Scrappy", max_pe);
    histograms_.fill("ecal_energy_event_6", total_ecal_energy);
    histograms_.fill("max_pe_vs_total_ecal_energy_event_6_Scrappy", total_ecal_energy, max_pe);
  }

  if ((run == 127) && (event_number == 246844)){
    histograms_.fill("max_pe_event_5_Scooby", max_pe);
    histograms_.fill("ecal_energy_event_5", total_ecal_energy);
    histograms_.fill("max_pe_vs_total_ecal_energy_event_5_Scooby", total_ecal_energy, max_pe);

  }

  if ((run == 170) && (event_number == 899016)){
    histograms_.fill("max_pe_event_4_Shaggy", max_pe);
    histograms_.fill("ecal_energy_event_4", total_ecal_energy);
    histograms_.fill("max_pe_vs_total_ecal_energy_event_4_Shaggy", total_ecal_energy, max_pe);

  }

  if ((run == 109) && (event_number == 273180)){
    histograms_.fill("max_pe_event_3_Velma", max_pe);
    histograms_.fill("ecal_energy_event_3", total_ecal_energy);
    histograms_.fill("max_pe_vs_total_ecal_energy_event_3_Velma", total_ecal_energy, max_pe);

  }

  if ((run == 134) && (event_number == 452830)){
    histograms_.fill("max_pe_event_2_Daphne", max_pe);
    histograms_.fill("ecal_energy_event_2", total_ecal_energy);
    histograms_.fill("max_pe_vs_total_ecal_energy_event_2_Daphne", total_ecal_energy, max_pe);

  }

  if ((run == 143) && (event_number == 903000)){
    histograms_.fill("max_pe_vs_total_ecal_energy_event_1_Fred", total_ecal_energy, max_pe);

    histograms_.fill("max_pe_event_1_Fred", max_pe);
    histograms_.fill("ecal_energy_event_1", total_ecal_energy);
  }









  //back to tracking
  //
  //looking at EcalScroingPlaneHits - we are under the hood of PEFF
  const auto& hits{event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits", "")}; // this is closed - can stay here

  auto thresh = 6; //define a threshold for sufficient energy at ECAL - can move this to a more relevant position.

  //creating a list of sorted hits
  std::vector<const ldmx::SimTrackerHit*> sorted_hits;
  sorted_hits.reserve(hits.size());
  for (const auto& hit: hits){
    ldmx::SimSpecialID hit_id = (hit.getID());
    // skip hits that are not particles _enterting_ (positive z-momentum)
    // the _front_ (plane == 31)
    if (hit_id.plane() != 31 or hit.getMomentum().at(2) < 0) {
      continue;
    }
    sorted_hits.emplace_back(&hit);
  } //list created

  std::sort(
      sorted_hits.begin(),
      sorted_hits.end(),
      [](const ldmx::SimTrackerHit* lhs, const ldmx::SimTrackerHit* rhs) {
      return mag(lhs->getMomentum()) > mag(rhs->getMomentum());
      }
      ); // list sorted

  //still within the analyzer but nothing else

  // 1: find highest momentum/energy electron
  const ldmx::SimTrackerHit* leading_electron{nullptr};
  for (const auto* hit: sorted_hits) {
    if (hit->getPdgID() == 11) {
      leading_electron = hit;
      break;
    }
  }
  if (leading_electron == nullptr) {
    no_leading_electron_count ++;
    //what is the leading particle??
    return;
  }
  auto leading_electron_energy = (leading_electron->getEnergy())/1000;



  // Now creating the Tracks collection - this is where I have Reco and Sim Momentum - i should be out of everything except the analyse call
  const auto& tracks{event.getCollection<ldmx::Track>("RecoilTracksClean", "")};

  for (const auto& trk: tracks) {
    auto track_at_ecal{trk.getTrackState(ldmx::TrackStateType::AtECAL)};
    if (not track_at_ecal) {
      continue;
    }
  }
  for (const auto& trk: tracks) {
    auto M_mag = mag(trk.getMomentum());
    histograms_.fill("reco_momentum", M_mag);
  }

  //i want the leading track - the one with the highest momentum

  auto sorted_tracks{sortByMomentum(tracks)};

  //only looking at tracks that actually have stuff in them 
  if (sorted_tracks.size() != 0) {
    const auto& leadtrk{*(sorted_tracks[0])};

    //LEAD TRACK STUFF

    //picked the largest momentum track
    auto leadtrk_at_ecal{leadtrk.getTrackState(ldmx::TrackStateType::AtECAL)};

    //only looking a those that actually show up at the ECAL
    if (leadtrk_at_ecal) {

      auto leadtrk_momentum = mag(leadtrk.getMomentum());
      histograms_.fill("reco_leadtrk_momentum", leadtrk_momentum);

      for (const auto* hit: sorted_hits) {

        // skip our leading electron
        if (hit == leading_electron) {

          //we've defined the leading electron, this is where we can define the momentum of the primary electron at the ECal front face - we use the ECal Scoring Plane Hit - which has momentum
          auto hit_momentum = hit->getMomentum();
          auto hpx = hit_momentum[0];
          auto hpy = hit_momentum[1];
          auto hpz = hit_momentum[2];
          const std::vector<double> ESPHp = {hpx,hpy,hpz};
          auto ESPHp_mag = mag(ESPHp)/1000; //both reco and sim in GeV now
          auto lead_electron_energy = (hit->getEnergy())/1000;
          histograms_.fill("esph_momentum",ESPHp_mag);
          histograms_.fill("leadtrk_reco_vs_esph_momentum",ESPHp_mag,leadtrk_momentum);

        }
      }


      auto QoP = leadtrk.getQoP();
      auto charge = QoP*leadtrk_momentum;

      histograms_.fill("reco_leadtrk_momentum_all_reqs", leadtrk_momentum);
      //histograms_.fill("leadtrk_all_req_reduced_chi2", chi2/4);

      GoodTracks ++;

      // ECAL Scoring plane stuff
      //// 2: loop through hits again and collect photons that are within the beamspot
      double nearby_energy{0.0};


      //create a used id list to not double count circling particles
      std::set<int> used_ids;

      for (const auto* hit: sorted_hits) {

        // skip our leading electron
        if (hit == leading_electron) {
          used_ids.insert(hit->getTrackID()); //add the leading electrons ID to the used id list
          continue;
        } //close leading electron skip


        auto leading_electron_pos = leading_electron->getPosition();
        // could filter for things here
        if ((hit->getPdgID() == 11) or (hit->getPdgID() == -11) or (hit->getPdgID() == 22)) {
          const auto& pos = hit->getPosition();

          bool inImpactRegion = (
              (pos[0] >= -20 && pos[0] <= 20) &&
              (pos[1] >= -50 && pos[1] <= 50)
              ); // nearby energy is whatever energy is in this region

          if (inImpactRegion and used_ids.find(hit->getTrackID())==used_ids.end()) {
            used_ids.insert(hit->getTrackID());
            nearby_energy += (hit->getEnergy()/1000);
          }
        }
      } //close sorted hits & adding nearby energy

      auto leading_electron_energy = (leading_electron->getEnergy())/1000;
      auto energy_estimate = leading_electron_energy + nearby_energy ;

      histograms_.fill("leadtrk_reco_all_reqs_vs_Energy_Estimate_Beamspot", energy_estimate, leadtrk_momentum);
      histograms_.fill("energy_estimate_vs_ecal_energy",energy_estimate, total_ecal_energy);
      histograms_.fill("leadtrk_reco_all_reqs_vs_ecal_energy", total_ecal_energy, leadtrk_momentum);
      if (max_pe < 10) {
        histograms_.fill("leadtrk_reco_all_reqs_vs_ecal_energy_PE_cuts", total_ecal_energy, leadtrk_momentum);
      }
      if ((leadtrk_momentum > 6) && (total_ecal_energy < 3)) {
        zone0_count ++;
        histograms_.fill("leadtrk_reco_all_reqs_vs_ecal_energy_zone_0", total_ecal_energy, leadtrk_momentum);
        histograms_.fill("ecal_energy_zone_0", total_ecal_energy);

      } else { 
        histograms_.fill("leadtrk_reco_all_reqs_vs_ecal_energy_zone_else", total_ecal_energy, leadtrk_momentum);
      }

      int run = event.getEventHeader().getRun();
      int event_number = event.getEventNumber();
      std::pair <int, int> ID = {run, event_number};
      if (IDs.find(ID) == IDs.end()){
        IDs.insert({run, event_number});
        std::cout << "Run No. " << run << " Event No. " << event_number << std::endl;
        std::cout << "Lead Track Reco Momentum = " << leadtrk_momentum << " GeV" << std::endl;
        std::cout << "Ecal Energy = " << total_ecal_energy << " GeV" << std::endl;
        for (const auto& [track_id, particle]: event.getMap<int, ldmx::SimParticle>("SimParticles", "simtrack")) {
          auto particle_pdg = particle.getPdgID() ;
          auto particle_energy = particle.getEnergy()/1000;
          auto particle_daughters = particle.getDaughters();
          if (particle_energy < 50) {
            std::cout << track_id << " -> PDG = " << particle_pdg
              << " Generated at = ("
              << particle.getVertex()[0] << ", "
              << particle.getVertex()[1] << ", "
              << particle.getVertex()[2] << ") with momentum = ("
              << particle.getMomentum()[0] << ", "
              << particle.getMomentum()[1] << ", "
              << particle.getMomentum()[2] << ") GeV and energy = " << particle.getEnergy()/1000 << " GeV" << std::endl;
            std::cout << "Daughters Track IDs: ";
for (const int id : particle_daughters) {
    std::cout << id << ", ";
}
std::cout << std::endl;
          }
        }
      }
    }//exit lead track at ECal loop
  }// close sorted tracks 

  //WHAT'S NEW SCOOBY DOO!!!!!! -- ZONE 0 EVENTS IN DEPTH
} //close analyser

DECLARE_ANALYZER(ZoneZeroTrackAnalyzer);
