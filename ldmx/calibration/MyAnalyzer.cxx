#include "Framework/EventProcessor.h"

#include "Ecal/Event/EcalHit.h"

#include "DetDescr/EcalID.h"

class MyAnalyzer : public framework::Analyzer {
 public:
  MyAnalyzer(const std::string& name, framework::Process& p)
    : framework::Analyzer(name, p) {}
  ~MyAnalyzer() = default;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;
};

void MyAnalyzer::onProcessStart() {
  getHistoDirectory();
  // this is where we will define the histograms we want to fill
  histograms_.create(
    "total_ecal_rec_energy",
    "Total ECal Rec Energy / GeV", 160, 0.0, 16.0
  );
}

void MyAnalyzer::analyze(const framework::Event& event) {
  const auto& ecal_rec_hits{event.getCollection<ldmx::EcalHit>("EcalRecHits")};
  // std::vector<ldmx::EcalHit>
  double total = 0.0;
  for (const auto& hit : ecal_rec_hits) {
    // convert MeV of hit energy to GeV for histogram
    ldmx::EcalID id{static_cast<unsigned int>(hit.getID())};
    // id.cell(), id.module(), id.layer()
    total += hit.getEnergy() / 1000.;
    if (id.module() == 0) {
      // only hits in core module
    }
  }

  histograms_.fill("total_ecal_rec_energy", total);
}

DECLARE_ANALYZER(MyAnalyzer);

