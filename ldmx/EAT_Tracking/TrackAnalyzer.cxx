#include "Framework/EventProcessor.h"
#include "Tracking/Event/Track.h" 
#include "SimCore/Event/SimTrackerHit.h"
#include "DetDescr/SimSpecialID.h"
#include <iostream>
#include <optional>
#include <vector>

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
  public:
  TrackAnalyzer(const std::string& name, framework::Process& p)
    : framework::Analyzer(name, p) {}
  ~TrackAnalyzer() = default;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;
  void onProcessEnd() final {
    std::cout << "no leading electron = " << no_leading_electron_count << std::endl;
  }
};


void TrackAnalyzer::onProcessStart () {
  // define the histograms I want to fill
  getHistoDirectory(); 
  histograms_.create("event_tracks",
      "No. of Tracks per Event", 100, -0.5, 99.5);
  histograms_.create("n_hits",
      "No. of Hits for Lead Track", 100, -0.5, 13.5);
  histograms_.create("clean_event_tracks",
      "No. of Clean Tracks per Event", 100, -0.5, 10.5);
  //histograms_.create("hits_per_track",
  //"No. of Hits", 100, -0.5,15.5,
  //"No. of Tracks per Event", 100, -0.5, 99.5);
  histograms_.create("impact_point",
      "X [mm]", 200, -200.0, 200.0,
      "Y [mm]", 200, -200.0, 200.0);
  histograms_.create("leadtrk_impact_point",
      "X [mm]", 200, -200.0, 200.0,
      "Y [mm]", 200, -200.0, 200.0);
  histograms_.create("layerID",
      "Layer ID of Hits", 100, 0, 32);
  histograms_.create("pdgID",
      "PDG ID of Hits", 1000, 0, 50);

  histograms_.create("reco_momentum",
      "Reco Momentum (GeV)", 1000, 0, 10);
  histograms_.create("sim_momentum",
      "Sim Momentum (GeV)", 1000, 0, 10);


  histograms_.create("sim_momentum_impact_cuts",
      "Sim Momentum (GeV) with Impact Cuts ", 1000, 0, 10);
  histograms_.create("reco_leadtrk_momentum",
      "Leading Reco Momentum (GeV)",1000,0,10);
  histograms_.create("reco_leadtrk_momentum_impact_cuts",
      "Leading Reco Momentum (GeV) with Impact Point Cuts", 1000, 0, 10);
  histograms_.create("leadtrk_reco_vs_sim_momentum",
      "Sim Momentum (GeV)", 100, 0, 10,
      "Reco Momentum (GeV)", 100,0, 10);
  histograms_.create("leadtrk_reco_vs_sim_momentum_impact_cuts",
      "Sim Momentum (GeV)",100,0,10,
      "Reco Momentum (GeV)", 100,0, 10);
  histograms_.create("leadtrk_reco_vs_sim_momentum_nhits_10_impact_cuts",
      "Sim Momentum (GeV)",100,0,10,
      "Reco Momentum (GeV)", 100,0, 10);
  histograms_.create("leadtrk_reco_vs_sim_momentum_nhits_10",
      "Sim Momentum (GeV)", 100, 0, 10,
      "Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("leadtrk_reco_vs_sim_momentum_nhits_under_10",
      "Sim Momentum (GeV)", 100, 0, 10,
      "Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("leadtrk_chi2",
      "Chi2 of Lead Track",100, 0, 20);
  histograms_.create("leadtrk_chi2_nhits_10",
      "Chi2 of Lead Tracks with 10 Hits",100, 0, 20);
  histograms_.create("leadtrk_chi2_nhits_10_impact_cuts",
      "Chi2 of Lead Tracks with 10 Hits - impact point cuts",100, 0, 20);

  //creating histograms to investigate  the ECal scoring plane energy as the "true" "sim" momentum - replacing sim momentum in our graphs with the energy of the electron + photons nearby

  histograms_.create("EScorePlane_momentum",
      "Momentum at Ecal Scoring Plane (MeV)", 1000, 0, 10);
  histograms_.create("leading_electron_energy", "Leading Electron Energy (GeV)", 1000, 0, 10); // no radius involvment

  // R = 2 mm
  histograms_.create("leading_E_vs_lead_E_w_energy_nearby_R2",
      "Leading Electron Energy (GeV)", 1000, 0, 10,
      "Leading Electron Energy + Nearby Energy (within R= 2mm) (GeV)", 1000, 0, 10);
  //need a better phrase for Lead E and Nearby E
  histograms_.create("leadtrk_reco_vs_Lead_E_w_nearby_R2", "ECalSP Lead E + Nearby Energy (GeV)", 100, 0, 10,
      "Reco Momentum (GeV)", 100, 0, 10);
  //applying the hits = 10 cut
  histograms_.create("leadtrk_reco_nhits_10_vs_Lead_E_w_nearby_R2",
      "ECalSP Lead E + Nearby Energy (GeV)", 100, 0, 10,
      "Reco Momentum (GeV)", 100, 0, 10);

  //R=5 - creating them now, not filling them, probably should write a piece of code that goes through this list of R
  histograms_.create("leadtrk_reco_vs_Lead_E_w_nearby_R5", 
      "ECalSP Lead E + Nearby Energy (GeV)", 100, 0, 10,
      "Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("leading_E_vs_lead_E_w_energy_nearby_R5",
      "Leading Electron Energy (GeV)", 1000, 0, 10,
      "Leading Electron Energy + Nearby Energy (within R=5mm) (GeV)", 1000, 0, 10);
  histograms_.create("leadtrk_reco_nhits_10_vs_Lead_E_w_nearby_R5",
      "ECalSP Lead E + Nearby Energy (GeV)", 100, 0, 10,
      "Reco Momentum (GeV)", 100, 0, 10);


  //R= 10 
  histograms_.create("leadtrk_reco_vs_Lead_E_w_nearby_R10",
      "ECalSP Lead E + Nearby Energy (GeV)", 100, 0, 10,
      "Reco Momentum (GeV)", 100, 0, 10);
  histograms_.create("leading_E_vs_lead_E_w_energy_nearby_R10",
      "Leading Electron Energy (GeV)", 1000, 0, 10,
      "Leading Electron Energy + Nearby Energy (within R=10mm) (GeV)", 1000, 0, 10);
  histograms_.create("leadtrk_reco_nhits_10_vs_Lead_E_w_nearby_R10",
      "ECalSP Lead E + Nearby Energy (GeV)", 100, 0, 10,
      "Reco Momentum (GeV)", 100, 0, 10);



}

void TrackAnalyzer::analyze(const framework::Event& event) {

  //looking at EcalScroingPlaneHits - we are under the hood of PEFF
  const auto& hits{event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits", "")}; // this is closed - can stay here

  // radii list?? tech a vector
  std::vector<int> radii = {2,5,10};

  // construct a list of photon positions
  std::vector<std::array<double,3>> photon_positions;
  for (const auto& hit: hits){
    //auto layerID = hit.getLayerID();
    auto pID = hit.getPdgID();
    if (pID == 22) {
      auto pos = hit.getPosition();
      auto x = pos[0];
      auto y = pos[1];
      auto z = pos[2];
      photon_positions.push_back(std::array<double,3>{x, y, z});
    }
  }
  // end of photon position list - can stay here

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
    std::cout << "Never found an electron entering the ECal in event " << event.getEventNumber() << std::endl; 
    //progress bar of sorts - need to find a way to tidy this
    return;
  }
  auto leading_electron_energy = (leading_electron->getEnergy())/1000;
  histograms_.fill("leading_electron_energy", leading_electron_energy);

  //UNSURE HERE BIG WARNING FLAGS COME BACK
  //!!!!!!!!!!!!!
  //!!!!!!!!!!!!!!


  //// HELP
  /////

  //
  //

  // Now creating the Tracks collection - this is where I have Reco and Sim Momentum - i should be out of everything except the analyse call
  // these are not radius dependent, don't want to run this every time

  const auto& tracks{event.getCollection<ldmx::Track>("RecoilTracksClean", "")};
  // looks like: std::vector<ldmx::Track>
  // histograms_.fill("clean_event_tracks", tracks.size());
  for (const auto& trk: tracks) {
    auto track_at_ecal{trk.getTrackState(ldmx::TrackStateType::AtECAL)};
    if (not track_at_ecal) {
      continue;
    }
    auto [x,y] = getImpactPoint(track_at_ecal.value());
    // histograms_.fill("impact_point", x, y);
  }
  for (const auto& trk: tracks) {
    auto M_mag = mag(trk.getMomentum());
    // histograms_.fill("reco_momentum", M_mag);
    // this works auto pID = trk.getPdgID() ;
    // std::cout << pID;
  }
  auto px = event.getObject<double>("PEFFPx","");
  auto py = event.getObject<double>("PEFFPy","");
  auto pz = event.getObject<double>("PEFFPz","");
  const std::vector<double> peffp{px,py,pz};
  auto peffp_mag = mag(peffp)/1000; //both reco and sim in GeV now
  histograms_.fill("sim_momentum", peffp_mag);
  //i want the leading track - the one with the highest momentum
  //
  auto sorted_tracks{sortByMomentum(tracks)};

  //only looking at tracks that actually have stuff in them 
  if (sorted_tracks.size() != 0) {
    const auto& leadtrk{*(sorted_tracks[0])};

    //LEAD TRACK STUFF

    //picked the largest momentum track
    auto leadtrk_at_ecal{leadtrk.getTrackState(ldmx::TrackStateType::AtECAL)};
    //only looking a thtose that actually show up at the ECAL

    if (leadtrk_at_ecal) {
      auto [x,y] = getImpactPoint(leadtrk_at_ecal.value());
      histograms_.fill("leadtrk_impact_point", x, y);

      auto leadtrk_momentum = mag(leadtrk.getMomentum());
      histograms_.fill("reco_leadtrk_momentum", leadtrk_momentum);
      histograms_.fill("leadtrk_reco_vs_sim_momentum",peffp_mag,leadtrk_momentum);

      auto nhits = leadtrk.getNhits();
      auto chi2  = leadtrk.getChi2();

      histograms_.fill("leadtrk_chi2", chi2);
      histograms_.fill("n_hits", nhits);

      bool inImpactRegion = (x >= -11.5 && x <= 9) && (y >= -38.5 && y <= 38.5);
      bool hasEnoughHits  = (nhits >= 10);

      // before radius loop because that's irrelevant
      if (inImpactRegion) {
        histograms_.fill("reco_leadtrk_momentum_impact_cuts", leadtrk_momentum);
        histograms_.fill("leadtrk_reco_vs_sim_momentum_impact_cuts", peffp_mag, leadtrk_momentum);
        // only those within the impact region - do they have enough hits? if they do what's chis
        if (hasEnoughHits) {
          histograms_.fill("leadtrk_reco_vs_sim_momentum_nhits_10_impact_cuts", peffp_mag, leadtrk_momentum);
          histograms_.fill("leadtrk_chi2_nhits_10_impact_cuts", chi2);
        }
      }

      // now just by hits alone
      if (hasEnoughHits) {
        histograms_.fill("leadtrk_reco_vs_sim_momentum_nhits_10", peffp_mag, leadtrk_momentum);
        histograms_.fill("leadtrk_chi2_nhits_10", chi2);
      } else {
        histograms_.fill("leadtrk_reco_vs_sim_momentum_nhits_under_10", peffp_mag, leadtrk_momentum);
      }
      // stopped looking at hits

      // ECAL Scoring plane stuff - should I change my nesting? - put ECAL Stuff within this?
      for (int r: radii) {
        // 2: loop through hits again and collect photons that are within radius of this electron
        double nearby_energy{0.0};
        for (const auto* hit: sorted_hits) {
          // skip our leading electron
          if (hit == leading_electron) {
            continue;
          }
          // could filter for things here
          const auto& leading_electron_pos = leading_electron->getPosition();
          const auto& pos = hit->getPosition();
          auto dx = leading_electron_pos[0] - pos[0];
          auto dy = leading_electron_pos[1] - pos[1];
          if (dx*dx + dy*dy < r*r) {
            // are within our circule of too close
            nearby_energy += (hit->getEnergy()/1000);
          }
        }
        auto leading_electron_energy = (leading_electron->getEnergy())/1000;
        auto total_energy_within_R = leading_electron_energy + nearby_energy ;

        histograms_.fill(("leading_E_vs_lead_E_w_energy_nearby_R"+ std::to_string(r)).c_str(), leading_electron_energy, total_energy_within_R);
        // another lead track thinng, but nested in my for loop bc that's what I want
        histograms_.fill(("leadtrk_reco_vs_Lead_E_w_nearby_R"+std::to_string(r)).c_str(),total_energy_within_R,leadtrk_momentum);
        // last one - by hits - iflead track has more than  10 hits
        if  (hasEnoughHits) {
          histograms_.fill(("leadtrk_reco_nhits_10_vs_Lead_E_w_nearby_R"+std::to_string(r)).c_str(), total_energy_within_R, leadtrk_momentum);
        }
      } //exit radii loop
    }//exit lead track at ECal loop
  }// close sorted tracks 
 
  const auto& dtracks{event.getCollection<ldmx::Track>("RecoilTracks", "")};
      // looks like: std::vector<ldmx::Track>
      histograms_.fill("event_tracks", dtracks.size());


    // i want to figure out the no. of tracks per event
    //  RunHeader.description_.size()

} //close analyser


    DECLARE_ANALYZER(TrackAnalyzer);

