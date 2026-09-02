#include "Framework/EventProcessor.h"
#include "Tracking/Event/Track.h" 
#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/Event/SimParticle.h"
#include "DetDescr/SimSpecialID.h"
#include <iostream>
#include <optional>
#include <vector>
#include <random>

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

class TrackAnalyzer : public framework::Analyzer {
  int no_leading_electron_count = 0;
  //int danger_count_ = 0;
  //int estimate_too_big_count = 0;
  int zone1_count = 0;
  int zone1_total =0;
  int zone2_count = 0;
  int zone2_total = 0;
  int zone3_count = 0;
  int zone3_total = 0;
  int zone4_count = 0;
  int zone4_total =0;
  int skimzone_count = 0;
  int TrackNumber = 0;
  int GoodTracks = 0;
  public:
  TrackAnalyzer(const std::string& name, framework::Process& p)
    : framework::Analyzer(name, p) {}
  ~TrackAnalyzer() = default;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;
  void recordSkimZone(int& zone_count,double energy_estimate,double leadtrk_momentum) {
    zone_count ++;
    skimzone_count ++;
    setStorageHint(framework::hint_shouldKeep);
    histograms_.fill("skim_zone_leadtrk_reco_all_reqs_vs_energy_estimate_beamspot", energy_estimate,leadtrk_momentum);
  }
  void onProcessEnd() final {
    std::cout << "hits with no leading electron = " << no_leading_electron_count << std::endl;
    //std::cout << "total danger count = " << danger_count_ << std::endl;
  std::cout << "zone 1 count = "<< zone1_count << " out of "<< zone1_total <<" events"<<std::endl;
    std::cout << "zone 2 count = "<< zone2_count << " out of "<< zone2_total <<" events" << std::endl;
    std::cout << "zone 3 count = "<< zone3_count << " out of "<< zone3_total <<" events" << std::endl;
    std::cout << "zone 4 count = "<< zone4_count << " out of "<< zone4_total <<" events" << std::endl;
    std::cout << "total skimzone count = " << skimzone_count << std::endl;
    //std::cout << "total energy estimate too big count = " << estimate_too_big_count << std::endl;
    std::cout << "total lead tracks at ECal = " << TrackNumber << std::endl;
    std::cout << "Total Lead Tracks, negatively charged, within beamspot and with 10 hits = " << GoodTracks<< std::endl;
  }
};


void TrackAnalyzer::onProcessStart () {
  // define the histograms I want to fill
  getHistoDirectory(); 
  histograms_.create("impact_point",
      "X [mm]", 500, -200.0, 200.0,
      "Y [mm]", 500, -200.0, 200.0);
  histograms_.create("leadtrk_impact_point",
      "X [mm]", 500, -200.0, 200.0,
      "Y [mm]", 500, -200.0, 200.0);
  histograms_.create("leadtrk_nhits_10_impact_point",
      "X [mm]", 500, -200.0, 200.0,
      "Y [mm]", 500, -200.0, 200.0);
  histograms_.create("leadtrk_nhits_10_neg_charge_impact_point",
      "X [mm]", 500, -200.0, 200.0,
      "Y [mm]", 500, -200.0, 200.0);

  histograms_.create("reco_momentum",
      "Track Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("sim_momentum",
      "Primary Electron Sim Momentum (GeV)", 100, 0, 10);
  histograms_.create("esph_momentum",
      "Hit Momentum at ECal (GeV)", 100, 0, 10);


  histograms_.create("reco_leadtrk_momentum",
      "Lead Track Reco Momentum (GeV)",100,0,10);
  histograms_.create("reco_leadtrk_momentum_all_reqs",
      "Lead Track Reco Momentum (GeV) within Beamspot", 100, 0, 10);


  histograms_.create("leadtrk_reco_vs_sim_momentum",
      "Primary Electron Sim Momentum (GeV)", 100, 0, 10,
      "Lead Track Reco Momentum (GeV)", 100,0, 10);
  histograms_.create("leadtrk_reco_vs_esph_momentum",
      "Hit Momentum at ECal (GeV)", 100, 0, 10,
      "Lead Track Reco Momentum (GeV)", 100,0, 10);



  histograms_.create("leadtrk_reco_vs_lead_electron_energy",
      "Primary Electron Energy (GeV)",100,0,10,
      "Lead Track Reco Momentum (GeV)", 100,0, 10);

  histograms_.create("leadtrk_reco_vs_sim_momentum_all_reqs",
      "Primary Electron Sim Momentum (GeV)",100,0,10,
      "Lead Track Reco Momentum (GeV)", 100,0, 10);

  histograms_.create("leadtrk_chi2",
      "Chi2 of Lead Track",100, 0, 20);
  histograms_.create("leadtrk_chi2_nhits_10_impact_cuts",
      "Chi2 of Lead Tracks with 10 Hits within Impact Region",100, 0, 20);
  histograms_.create("leadtrk_all_req_reduced_chi2",
      "Chi2/NDF of Lead Track (ALL REQS)", 100, 0, 20);

  histograms_.create("leadtrk_reco_all_reqs_vs_Energy_Estimate_Beamspot",
      "Energy Estimate within Beamspot (GeV)", 100,0,10,
      "Lead Track Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("difference_Beamspot",
      "Energy Difference (Reco - Estimate (GeV)", 100,-10,10);
  histograms_.create("energy_diff_vs_chi2_Beamspot",
      "Chi2 of Lead Track", 100, 0, 20,
      "Energy Difference (Reco - Estimate)(GeV)", 100,-10,10);

  histograms_.create("skim_zone_leadtrk_reco_all_reqs_vs_energy_estimate_beamspot",
      "Energy Estimate within Beamspot (GeV)", 100, 0, 10,
      "Lead Track Reco Momentum (GeV)", 100, 0, 10);


}

void TrackAnalyzer::analyze(const framework::Event& event) {

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
    auto [x,y] = getImpactPoint(track_at_ecal.value());
    histograms_.fill("impact_point", x, y);
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
      TrackNumber++;

      auto [x,y] = getImpactPoint(leadtrk_at_ecal.value());
      histograms_.fill("leadtrk_impact_point", x, y);

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
          histograms_.fill("leadtrk_reco_vs_lead_electron_energy",lead_electron_energy, leadtrk_momentum);

        }
      }


      auto QoP = leadtrk.getQoP();
      auto charge = QoP*leadtrk_momentum;

      auto nhits = leadtrk.getNhits();
      auto chi2  = leadtrk.getChi2();
      histograms_.fill("leadtrk_chi2", chi2);

      bool inBeamspot = (x >= -11.5 && x <= 9) && (y >= -38.5 && y <= 38.5);
      bool hasEnoughHits  = (nhits >= 10);

      if (hasEnoughHits) {
        histograms_.fill("leadtrk_nhits_10_impact_point", x, y);
        if (charge < 0) {
          histograms_.fill("leadtrk_nhits_10_neg_charge_impact_point", x,y);
          if  (inBeamspot) {
            histograms_.fill("reco_leadtrk_momentum_all_reqs", leadtrk_momentum);
            histograms_.fill("leadtrk_all_req_reduced_chi2", chi2/4);

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
            auto difference = leadtrk_momentum - energy_estimate;

            histograms_.fill("leadtrk_reco_all_reqs_vs_Energy_Estimate_Beamspot", energy_estimate, leadtrk_momentum);
            histograms_.fill("difference_Beamspot", difference);
            histograms_.fill("energy_diff_vs_chi2_Beamspot", chi2, difference);

            std::random_device rd;  // Will be used to obtain a seed for the random number engine
            std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
            std::uniform_real_distribution<> dis(0.0, 1.0);
            auto rando = dis(gen);
            //std::cout << "Lead Track Reco = " << leadtrk_momentum << ", Energy Estimate = " << energy_estimate << " , random chance = " << rando << '\n';

            if (leadtrk_momentum > 1.25*(energy_estimate)) {
              if (energy_estimate < 5){
                zone1_total ++;
                recordSkimZone(zone1_count,energy_estimate, leadtrk_momentum);
              } else if (energy_estimate< 6) {
                  zone2_total ++;
                  if (rando < 0.5) {
                    recordSkimZone(zone2_count,energy_estimate,leadtrk_momentum);
                  }
              } else if (energy_estimate < 7){
                zone3_total ++;
                if  (rando < 0.25) {
                  recordSkimZone(zone3_count,energy_estimate,leadtrk_momentum );
                }
              } else {
                zone4_total ++;
                if (rando < 0.1) {
                  recordSkimZone(zone4_count,energy_estimate,leadtrk_momentum );
                }
              }
             } // end skim zone examination
            } // exit inBamspot loop
          }//exit charge requirement
        }//exit hit requirement
      }//exit lead track at ECal loop
    }// close sorted tracks 
  } //close analyser

  DECLARE_ANALYZER(TrackAnalyzer);
