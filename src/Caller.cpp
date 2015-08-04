/*******************************************************************************
 Copyright 2006-2012 Lukas Käll <lukas.kall@scilifelab.se>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 *******************************************************************************/

#include "Caller.h"
#ifndef _WIN32
  #include "unistd.h"
#endif
#include <iomanip>
#include <set>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

Caller::Caller() :
        pNorm_(NULL), pCheck_(NULL), protEstimator_(NULL),
        tabInputFN_(""), tabOutputFN_(""), psmResultFN_(""), 
        peptideResultFN_(""), proteinResultFN_(""), decoyPsmResultFN_(""), 
        decoyPeptideResultFN_(""), decoyProteinResultFN_(""),
        weightFN_(""), tabInput_(false), readStdIn_(false),
        reportUniquePeptides_(true), targetDecoyCompetition_(true),
        testFdr_(0.01), threshTestRatio_(0.3), trainRatio_(0.6) {
}

Caller::~Caller() {
  if (pNorm_) {
    delete pNorm_;
  }
  pNorm_ = NULL;
  if (pCheck_) {
    delete pCheck_;
  }
  pCheck_ = NULL;
  if (protEstimator_) {
    delete protEstimator_;
  }
  protEstimator_ = NULL;
}

string Caller::extendedGreeter() {
  ostringstream oss;
  char* host = getenv("HOSTNAME");
  oss << greeter();
  oss << "Issued command:" << endl << call_ << endl;
  oss << "Started " << ctime(&startTime_) << endl;
  oss.seekp(-1, ios_base::cur);
  if(host) oss << " on " << host << endl;
  crossValidation_.printParameters(oss);
  return oss.str();
}

string Caller::greeter() {
  ostringstream oss;
  oss << "Percolator version " << VERSION << ", ";
  oss << "Build Date " << __DATE__ << " " << __TIME__ << endl;
  oss << "Copyright (c) 2006-9 University of Washington. All rights reserved.\n"
      << "Written by Lukas Käll (lukall@u.washington.edu) in the\n"
      << "Department of Genome Sciences at the University of Washington.\n";
  return oss.str();
}

bool Caller::parseOptions(int argc, char **argv) {
  ostringstream callStream;
  callStream << argv[0];
  for (int i = 1; i < argc; i++) {
    callStream << " " << argv[i];
  }
  callStream << endl;
  call_ = callStream.str();
  call_ = call_.substr(0,call_.length()-1); // trim ending carriage return
  ostringstream intro, endnote;
  intro << greeter() << "\nUsage:\n";
  intro << "   percolator [-X pout.xml] [other options] pin.tsv\n";
  intro << "pin.tsv is the tab delimited output file generated by e.g. sqt2pin;\n";
  intro << "  The tab delimited fields should be:\n";
  intro << "    id <tab> label <tab> scannr <tab> feature1 <tab> ... <tab>\n";
  intro << "    featureN <tab> peptide <tab> proteinId1 <tab> .. <tab> proteinIdM\n";
  intro << "  Labels are interpreted as 1 -- positive set and test set, -1 -- negative set.\n";
  intro << "  When the --doc option the first and second feature should contain\n";
  intro << "  the retention time and difference between observed and calculated mass;\n";
  intro << "pout.xml is where the output will be written (ensure to have read\n";
  intro << "and write access on the file)." << std::endl;
  // init
  CommandLineParser cmd(intro.str());
  // available lower case letters: c, f, g, o, z
  // available upper case letters: I, N, Q 
  cmd.defineOption("X",
      "xmloutput",
      "Path to xml-output (pout) file.",
      "filename");
  cmd.defineOption("",
      "stdinput",
      "Read percolator tab-input format (pin-tab) from standard input",
      "",
      TRUE_IF_SET);
  cmd.defineOption("e",
      "stdinput-xml",
      "Read percolator xml-input format (pin-xml) from standard input",
      "",
      TRUE_IF_SET);
  cmd.defineOption("Z",
      "decoy-xml-output",
      "Include decoys (PSMs, peptides and/or proteins) in the xml-output. Only available if -X is set.",
      "",
      TRUE_IF_SET);
  cmd.defineOption("p",
      "Cpos",
      "Cpos, penalty for mistakes made on positive examples. Set by cross validation if not specified.",
      "value");
  cmd.defineOption("n",
      "Cneg",
      "Cneg, penalty for mistakes made on negative examples. Set by cross validation if not specified or if -p is not specified.",
      "value");
  cmd.defineOption("t",
      "testFDR",
      "False discovery rate threshold for evaluating best cross validation result and reported end result. Default = 0.01.",
      "value");
  cmd.defineOption("F",
      "trainFDR",
      "False discovery rate threshold to define positive examples in training. Set to testFDR if 0. Default = 0.01.",
      "value"); 
  cmd.defineOption("i",
      "maxiter",
      "Maximal number of iterations",
      "number");
  cmd.defineOption("x",
      "quick-validation",
      "Quicker execution by reduced internal cross-validation.",
      "",
      TRUE_IF_SET);
  /*
  cmd.defineOption("f",
      "train-ratio",
      "Fraction of the negative data set to be used as train set when only providing one negative set, \
      remaining examples will be used as test set. Set to 0.6 by default.",
      "value");
  */
  cmd.defineOption("J",
      "tab-out",
      "Output computed features to given file in pin-tab format.",
      "filename");
  cmd.defineOption("j",
      "tab-in [default]",
      "Input file given in pin-tab format. This is the default setting, flag only present for backwards compatibility.",
      "filename");
  cmd.defineOption("k",
      "xml-in",
      "Input file given in deprecated pin-xml format generated by e.g. sqt2pin with the -k option",
      "filename");
  cmd.defineOption("w",
      "weights",
      "Output final weights to given file",
      "filename");
  cmd.defineOption("W",
      "init-weights",
      "Read initial weights from given file (one per line)",
      "filename");
  cmd.defineOption("V",
      "default-direction",
      "Use given feature name as initial search direction, can be negated to indicate that a lower value is better.",
      "[-]?featureName");
  cmd.defineOption("v",
      "verbose",
      "Set verbosity of output: 0=no processing info, 5=all. Default = 2",
      "level");
  cmd.defineOption("u",
      "unitnorm",
      "Use unit normalization [0-1] instead of standard deviation normalization",
      "",
      TRUE_IF_SET);
  cmd.defineOption("R",
      "test-each-iteration",
      "Measure performance on test set each iteration",
      "",
      TRUE_IF_SET);
  cmd.defineOption("O",
      "override",
      "Override error check and do not fall back on default score vector in case of suspect score vector from SVM.",
      "",
      TRUE_IF_SET);
  cmd.defineOption("S",
      "seed",
      "Set seed of the random number generator. Default = 1",
      "value");
  cmd.defineOption("D",
      "doc",
      "Include description of correct features, i.e. features describing the difference between the observed and predicted isoelectric point, retention time and precursor mass.",
      "",
      MAYBE,
      "15");
  cmd.defineOption("K",
      "klammer",
      "Retention time features are calculated as in Klammer et al. Only available if -D is set.",
      "",
      TRUE_IF_SET);
  cmd.defineOption("r",
      "results-peptides",
      "Output tab delimited results of peptides to a file instead of stdout (will be ignored if used with -U option)",
      "filename");
  cmd.defineOption("m",
      "results-psms",
      "Output tab delimited results of PSMs to a file instead of stdout",
      "filename");
  cmd.defineOption("B",
      "decoy-results-peptides",
      "Output tab delimited results for decoy peptides into a file (will be ignored if used with -U option)",
      "filename");
  cmd.defineOption("M",
      "decoy-results-psms",
      "Output tab delimited results for decoy PSMs into a file",
      "filename");
  cmd.defineOption("U",
      "only-psms",
      "Do not remove redundant peptides, keep all PSMS and exclude peptide level probabilities.",
      "",
      FALSE_IF_SET);
  cmd.defineOption("y",
      "post-processing-qvality",
      "Replace the target-decoy competition with the method qvality to assign q-values and PEPs. Note that this option only has an effect if the input PSMs are from separate target and decoy searches.",
      "",
      TRUE_IF_SET);
  cmd.defineOption("Y",
      "post-processing-tdc [default]",
      "Use target-decoy competition to assign q-values and PEPs. This is the default setting, flag is only present for backwards compatibility.",
      "",
      TRUE_IF_SET);
  cmd.defineOption("s",
      "no-schema-validation",
      "Skip validation of input file against xml schema.",
      "",
      TRUE_IF_SET);
  cmd.defineOption("A",
      "protein",
      "Use the Fido algorithm to infer protein probabilities",
      "",
      TRUE_IF_SET);
  cmd.defineOption("a",
      "fido-alpha",
      "Set Fido's probability with which a present protein emits an associated peptide. \
       Set by grid search if not specified.",
      "value");
  cmd.defineOption("b",
      "fido-beta",
      "Set Fido's probability of creation of a peptide from noise. Set by grid search if not specified",
      "value");
  cmd.defineOption("G",
      "fido-gamma",
      "Set Fido's prior probability that a protein is present in the sample. Set by grid search if not specified",
      "value");
  cmd.defineOption("I",
      "fido-protein-level-pi0",
      "The pi_0 value when calculating protein-level empirical q-values. The value defaults to 1.0",
      "value");
  cmd.defineOption("q",
      "fido-empirical-protein-q",        
      "Estimate empirical p-values and q-values using target-decoy analysis.",
      "",
      TRUE_IF_SET);
  cmd.defineOption("P",
      "fido-pattern",
      "Define the text pattern to identify decoy proteins in the database. Default = \"random\".",
      "value");
  cmd.defineOption("d",
      "fido-gridsearch-depth",
      "Setting the gridsearch-depth to 0 (fastest), 1 or 2 (slowest) controls how much computational time is required for the estimation of alpha, beta and gamma parameters for Fido. Default = 0.",
      "value");
  cmd.defineOption("T",
      "fido-fast-gridsearch",
      "Apply the specified threshold to PSM, peptide and protein probabilities to obtain a faster estimate of the alpha, beta and gamma parameters. Default = 0; Recommended when set = 0.2.",
      "value");
  cmd.defineOption("C",
      "fido-no-split-large-components",        
      "Do not approximate the posterior distribution by allowing large graph components to be split into subgraphs. The splitting is done by duplicating peptides with low probabilities. Splitting continues until the number of possible configurations of each subgraph is below 2^18.",
      "",
      TRUE_IF_SET);
  cmd.defineOption("E",
      "fido-protein-truncation-threshold",
      "To speed up inference, proteins for which none of the associated peptides has a probability exceeding the specified threshold will be assigned probability = 0. Default = 0.01.",
      "value");
  cmd.defineOption("H",
      "fido-gridsearch-mse-threshold",
      "Q-value threshold that will be used in the computation of the MSE and ROC AUC score in the grid search. Recommended 0.05 for normal size datasets and 0.1 for big size datasets. Default = 0.1",
      "value");
  /*
  cmd.defineOption("Q",
      "fido-protein-group-level-inference",
      "Uses protein group level inference, each cluster of proteins is either present or not, therefore when grouping proteins discard all possible combinations for each group.(Only valid if option -A is active and -N is inactive).",
      "",
      TRUE_IF_SET);
  */
  cmd.defineOption("l",
      "results-proteins",
      "Output tab delimited results of proteins to a file instead of stdout (Only valid if option -A is active)",
      "filename");
  cmd.defineOption("L",
      "decoy-results-proteins",
      "Output tab delimited results for decoy proteins into a file (Only valid if option -A is active)",
      "filename");
  
  // finally parse and handle return codes (display help etc...)
  cmd.parseArgs(argc, argv);
  
  if (cmd.optionSet("v")) {
    Globals::getInstance()->setVerbose(cmd.getInt("v", 0, 10));
  }
  
  // now query the parsing results
  if (cmd.optionSet("X")) xmlInterface_.setXmlOutputFN(cmd.options["X"]);
  
  // filenames for outputting results to file
  if (cmd.optionSet("m"))  psmResultFN_ = cmd.options["m"];
  if (cmd.optionSet("M"))  decoyPsmResultFN_ = cmd.options["M"];
  
  if (cmd.optionSet("U")) {
    if (cmd.optionSet("A")){
      cerr
      << "ERROR: The -U option cannot be used in conjunction with -A: peptide level statistics\n"
      << "are needed to calculate protein level ones.";
      return 0;
    }
    if (cmd.optionSet("r")) {
      if (!cmd.optionSet("m")) {
        if (VERB > 0) {
          cerr
          << "WARNING: The -r option cannot be used in conjunction with -U: no peptide level statistics\n"
          << "are calculated, redirecting PSM level statistics to provided file instead." << endl;
        }
        psmResultFN_ = cmd.options["r"];
      } else {
        cerr
        << "WARNING: The -r option cannot be used in conjunction with -U: no peptide level statistics\n"
        << "are calculated, ignoring -r option." << endl;
      }
    }
    if (cmd.optionSet("B")) {
      if (!cmd.optionSet("M")) {
        if (VERB > 0) {
          cerr
          << "WARNING: The -B option cannot be used in conjunction with -U: no peptide level statistics\n"
          << "are calculated, redirecting decoy PSM level statistics to provided file instead." << endl;
        }
        decoyPsmResultFN_ = cmd.options["B"]; 
      } else {
        cerr
        << "WARNING: The -B option cannot be used in conjunction with -U: no peptide level statistics\n"
        << "are calculated, ignoring -B option." << endl;
      }
    }
    reportUniquePeptides_ = false;
  } else {
    if (cmd.optionSet("r"))  peptideResultFN_ = cmd.options["r"];
    if (cmd.optionSet("B"))  decoyPeptideResultFN_ = cmd.options["B"];
  }

  if (cmd.optionSet("A")) {
  
    ProteinProbEstimator::setCalcProteinLevelProb(true);
    
    /*fido parameters*/
    
    // General Fido options
    double fidoAlpha = -1;
    double fidoBeta = -1;
    double fidoGamma = -1;
    if (cmd.optionSet("a")) fidoAlpha = cmd.getDouble("a", 0.00, 1.0);
    if (cmd.optionSet("b")) fidoBeta = cmd.getDouble("b", 0.00, 1.0);
    if (cmd.optionSet("G")) fidoGamma = cmd.getDouble("G", 0.00, 1.0);
    
    // Confidence estimation options (general protein prob options)
    bool protEstimatorUsePi0 = true; // cannot be set on cmd line, but pi_0 value can
    bool protEstimatorOutputEmpirQVal = false;
    bool protEstimatorTrivialGrouping = true; // cannot be set on cmd line
    std::string protEstimatorDecoyPrefix = "random";
    double  protEstimatorPi0 = 1.0;
    if (cmd.optionSet("I")) protEstimatorPi0 = cmd.getDouble("I", 0.0, 1.0);
    protEstimatorOutputEmpirQVal = cmd.optionSet("q");
    if (cmd.optionSet("P")) protEstimatorDecoyPrefix = cmd.options["P"];
    //if (cmd.optionSet("Q")) protEstimatorTrivialGrouping = false;
    
    // Options for controlling speed
    bool fidoNoPartitioning = false; // cannot be set on cmd line
    bool fidoNoClustering = false; // cannot be set on cmd line
    unsigned fidoGridSearchDepth = 0;
    bool fidoNoPruning = false;
    double fidoGridSearchThreshold = 0.0;
    double fidoProteinThreshold = 0.01;
    double fidoMseThreshold = 0.1;
    if (cmd.optionSet("d")) fidoGridSearchDepth = cmd.getInt("d", 0, 2);
    if (cmd.optionSet("T")) fidoGridSearchThreshold = cmd.getDouble("T", 0.0, 1.0);
    if (cmd.optionSet("C")) fidoNoPruning = true;
    if (cmd.optionSet("E")) fidoProteinThreshold = cmd.getDouble("T", 0.0, 1.0);
    if (cmd.optionSet("H")) fidoMseThreshold = cmd.getDouble("H",0.001,1.0);
    
    // Output file options
    if (cmd.optionSet("l")) proteinResultFN_ = cmd.options["l"];
    if (cmd.optionSet("L")) decoyProteinResultFN_ = cmd.options["L"];
    
    protEstimator_ = new FidoInterface(fidoAlpha, fidoBeta, fidoGamma, 
              fidoNoClustering, fidoNoPartitioning, fidoNoPruning,
              fidoGridSearchDepth, fidoGridSearchThreshold,
              fidoProteinThreshold, fidoMseThreshold,
              protEstimatorPi0, protEstimatorOutputEmpirQVal, 
              protEstimatorDecoyPrefix, protEstimatorTrivialGrouping);
  }
  
  if (cmd.optionSet("k")) {
    tabInput_ = false;
    xmlInterface_.setXmlInputFN(cmd.options["k"]);
  }
  
  if (cmd.optionSet("e")) {
    readStdIn_ = true;
    tabInput_ = false;
  }
  
  if (cmd.optionSet("j")) {
    tabInput_ = true;
    tabInputFN_ = cmd.options["j"];
  }
  
  if (cmd.optionSet("")) {
    readStdIn_ = true;
    tabInput_ = true;
  }
  
  if (cmd.optionSet("p")) {
    crossValidation_.setSelectedCpos(cmd.getDouble("p", 0.0, 1e127));
  }
  if (cmd.optionSet("n")) {
    crossValidation_.setSelectedCneg(cmd.getDouble("n", 0.0, 1e127));
    if (crossValidation_.getSelectedCpos() == 0) {
      std::cerr << "WARNING: the positive penalty(cpos) is 0, therefore both the "  
               << "positive and negative penalties are going "
               << "to be cross-validated. The option --Cneg has to be used together "
               << "with the option --Cpos" << std::endl;
    }
  }
  if (cmd.optionSet("J")) {
    tabOutputFN_ = cmd.options["J"];
  }
  
  if (cmd.optionSet("w")) {
    weightFN_ = cmd.options["w"];
  }
  if (cmd.optionSet("W")) {
    SanityCheck::setInitWeightFN(cmd.options["W"]);
  }
  if (cmd.optionSet("V")) {
    SanityCheck::setInitDefaultDirName(cmd.options["V"]);
  }
  /*
  if (cmd.optionSet("f")) {
    trainRatio_ = cmd.getDouble("f", 0.0, 1.0);
  }
  */
  if (cmd.optionSet("u")) {
    Normalizer::setType(Normalizer::UNI);
  }
  if (cmd.optionSet("O")) {
    SanityCheck::setOverrule(true);
  }
  if (cmd.optionSet("R")) {
    crossValidation_.setReportPerformanceEachIteration(true);
  }
  if (cmd.optionSet("x")) {
    crossValidation_.setQuickValidation(true);
  }
  if (cmd.optionSet("F")) {
    crossValidation_.setSelectionFdr(cmd.getDouble("F", 0.0, 1.0));
  }
  if (cmd.optionSet("t")) {
    testFdr_ = cmd.getDouble("t", 0.0, 1.0);
    crossValidation_.setTestFdr(testFdr_);
  }
  if (cmd.optionSet("i")) {
    crossValidation_.setNiter(cmd.getInt("i", 0, 1000));
  }
  if (cmd.optionSet("S")) {
    Scores::setSeed(cmd.getInt("S", 1, 20000));
  }
  if (cmd.optionSet("D")) {
    DataSet::setCalcDoc(true);
    DescriptionOfCorrect::setDocType(cmd.getInt("D", 0, 15));
  }
  if (cmd.optionSet("K")) {
    DescriptionOfCorrect::setKlammer(true);
  }
  if (cmd.optionSet("Z")) {
    Scores::setPrintDecoysInXml(true);
  }
  if (cmd.optionSet("s")) {
    xmlInterface_.setSchemaValidation(false);
  }
  Scores::setShowExpMass(true);
  if (cmd.optionSet("y")) {
    if (cmd.optionSet("A")) {
      std::cerr << "WARNING: cannot use qvality for pep calculation when predicting protein probabilities with the -A flag, ignoring the -y flag." << std::endl;
    } else {
      targetDecoyCompetition_ = false;
      Scores::setUsePi0(true);
    }
  }
  // if there are no arguments left...
  if (cmd.arguments.size() == 0) {
    if(!cmd.optionSet("j") && !cmd.optionSet("k") && !cmd.optionSet("e") && !cmd.optionSet("")){ // unless the input comes from -j, -k or -e option
      cerr << "Error: too few arguments.";
      cerr << "\nInvoke with -h option for help\n";
      return 0; // ...error
    }
  }
  // if there is one argument left...
  if (cmd.arguments.size() == 1) {
    tabInput_ = true;
    tabInputFN_ = cmd.arguments[0]; // then it's the pin input
    if (cmd.optionSet("k") || cmd.optionSet("j")){ // and if the tab input is also present
      cerr << "Error: use one of either pin-xml or tab-delimited input format.";
      cerr << "\nInvoke with -h option for help.\n";
      return 0; // ...error
    }
    if (cmd.optionSet("e") || cmd.optionSet("")){ // if stdin pin file is present
      cerr << "Error: the pin file has already been given as stdinput argument.";
      cerr << "\nInvoke with -h option for help.\n";
      return 0; // ...error
    }
  }
  // if there is more then one argument left...
  if (cmd.arguments.size() > 1) {
    cerr << "Error: too many arguments.";
    cerr << "\nInvoke with -h option for help\n";
    return 0; // ...error
  }

  return true;
}

/**
 * Reads in the files from XML (must be enabled at compile time) or tab format
 */
int Caller::readFiles() { 
  int error = 0;
  if (!tabInput_) {
    std::ifstream fileStream;
    if (!readStdIn_) {
      fileStream.exceptions(ifstream::badbit | ifstream::failbit);
      fileStream.open(xmlInterface_.getXmlInputFN().c_str(), ios::in);
      if (VERB > 1) {
        std::cerr << "Reading XML input from datafile " << 
                     xmlInterface_.getXmlInputFN() << std::endl;
      }
    }
    
    std::istream &dataStream = readStdIn_ ? std::cin : fileStream;
    
    error = xmlInterface_.readPin(dataStream, setHandler_, pCheck_, protEstimator_);
  } else {
    std::ifstream fileStream;
    if (!readStdIn_) {
      fileStream.open(tabInputFN_.c_str(), ios::in);
      if (VERB > 1) {
        std::cerr << "Reading Tab delimited input from datafile " << 
                     tabInputFN_ << std::endl;
      }
    }
    std::istream &dataStream = readStdIn_ ? std::cin : fileStream;
    
    error = setHandler_.readTab(dataStream, pCheck_);
  }
  return error;
}


/** 
 * Fills in the features previously read from file and normalizes them
 */
void Caller::fillFeatureSets() {
  allScores_.fillFeatures(setHandler_);
  if (VERB > 1) {
    cerr << "Train/test set contains " << allScores_.posSize()
        << " positives and " << allScores_.negSize()
        << " negatives, size ratio=" << allScores_.getTargetDecoySizeRatio()
        << " and pi0=" << allScores_.getPi0() << endl;
  }
  
  // check for the minimum recommended number of positive and negative hits
  if (allScores_.posSize() <= (unsigned)(FeatureNames::getNumFeatures() * 5)) {
    std::cerr << "Warning : the number of positive samples read is too small to perform a correct classification.\n" << std::endl;
  }
  if (allScores_.negSize() <= (unsigned)(FeatureNames::getNumFeatures() * 5)) {
    std::cerr << "Warning : the number of negative samples read is too small to perform a correct classification.\n" << std::endl;
  }
  
  if (DataSet::getCalcDoc()) {
    for (std::vector<DataSet*>::iterator it = setHandler_.getSubsets().begin();
         it != setHandler_.getSubsets().end(); ++it) {
      (*it)->setRetentionTime(scanToRetentionTimeMap_);
    }
  }
  
  //Normalize features
  vector<double*> featuresV, rtFeaturesV;
  for (std::vector<DataSet*>::iterator it = setHandler_.getSubsets().begin();
         it != setHandler_.getSubsets().end(); ++it) {
    (*it)->fillFeatures(featuresV);
    (*it)->fillRtFeatures(rtFeaturesV);
  }
  pNorm_ = Normalizer::getNormalizer();

  pNorm_->setSet(featuresV,
      rtFeaturesV,
      FeatureNames::getNumFeatures(),
      DataSet::getCalcDoc() ? RTModel::totalNumRTFeatures() : 0);
  pNorm_->normalizeSet(featuresV, rtFeaturesV);
}


/** Calculates the PSM and/or peptide probabilities
 * @param isUniquePeptideRun boolean indicating if we want peptide or PSM probabilities
 * @param procStart clock time when process started
 * @param procStartClock clock associated with procStart
 * @param diff runtime of the calculations
 */
void Caller::calculatePSMProb(bool isUniquePeptideRun, time_t& procStart,
    clock_t& procStartClock, double& diff){
  // write output (cerr or xml) if this is the unique peptide run and the
  // reportUniquePeptides_ option was switched on OR if this is not the unique
  // peptide run and the option was switched off
  bool writeOutput = (isUniquePeptideRun == reportUniquePeptides_);
  
  if (reportUniquePeptides_ && VERB > 0 && writeOutput) {
    cerr << "Tossing out \"redundant\" PSMs keeping only the best scoring PSM "
        "for each unique peptide." << endl;
  }
  
  if (isUniquePeptideRun) {
    allScores_.weedOutRedundant();
  } else if (targetDecoyCompetition_) {
    allScores_.weedOutRedundantTDC();
    if (VERB > 0) {
      std::cerr << "Target Decoy Competition yielded " << allScores_.posSize() 
        << " target PSMs and " << allScores_.negSize() << " decoy PSMs" << std::endl;
    }
  }
  
  if (VERB > 0 && writeOutput) {
    if (!targetDecoyCompetition_)
      std::cerr << "Selecting pi_0=" << allScores_.getPi0() << std::endl;
    std::cerr << "Calibrating statistics - calculating q values" << std::endl;
  }
  
  int foundPSMs = allScores_.calcQ(testFdr_);
  allScores_.calcPep();
  
  if (VERB > 0 && DataSet::getCalcDoc() && writeOutput) {
    crossValidation_.printDOC();
  }
  
  if (VERB > 0 && writeOutput) {
    if (targetDecoyCompetition_) {
      std::cerr << "Merged list gives ";
    } else {
      std::cerr << "New pi_0 estimate on merged list gives ";
    }
    std::cerr << foundPSMs << (reportUniquePeptides_ ? " peptides" : " PSMs") 
        << " over q=" << testFdr_ << endl;
  }
  
  if (VERB > 0 && writeOutput) {
    cerr
    << "Calibrating statistics - calculating Posterior error probabilities (PEPs)"
    << endl;
  }
  
  time_t end;
  time(&end);
  diff = difftime(end, procStart);
  ostringstream timerValues;
  timerValues.precision(4);
  timerValues << "Processing took " << ((double)(clock() - procStartClock)) / (double)CLOCKS_PER_SEC
              << " cpu seconds or " << diff << " seconds wall time" << endl;
  
  if (VERB > 1 && writeOutput) {
    std::cerr << timerValues.str();
  }
  
  if (weightFN_.size() > 0) {
    ofstream weightStream(weightFN_.data(), ios::out);
    crossValidation_.printAllWeights(weightStream, pNorm_);
    weightStream.close();
  }
  
  if (isUniquePeptideRun) {
    if (peptideResultFN_.empty()) {
      setHandler_.print(allScores_, NORMAL);
    } else {
      ofstream targetStream(peptideResultFN_.data(), ios::out);
      setHandler_.print(allScores_, NORMAL, targetStream);
    }
    if (!decoyPeptideResultFN_.empty()) {
      ofstream decoyStream(decoyPeptideResultFN_.data(), ios::out);
      setHandler_.print(allScores_, SHUFFLED, decoyStream);
    }
    // set pi_0 value (to be outputted)
    xmlInterface_.setPi0Peptides(allScores_.getPi0());
  } else {
    if (psmResultFN_.empty() && writeOutput) {
      setHandler_.print(allScores_, NORMAL);
    } else if (!psmResultFN_.empty()) {
      ofstream targetStream(psmResultFN_.data(), ios::out);
      setHandler_.print(allScores_, NORMAL, targetStream);
    }
    if (!decoyPsmResultFN_.empty()) {
      ofstream decoyStream(decoyPsmResultFN_.data(), ios::out);
      setHandler_.print(allScores_, SHUFFLED, decoyStream);
    }
    // set pi_0 value (to be outputted)
    xmlInterface_.setPi0Psms(allScores_.getPi0());
    xmlInterface_.setNumberQpsms(allScores_.getQvaluesBelowLevel(0.01));
  }
}

/** 
 * Calculates the protein probabilites by calling Fido and directly writes 
 * the results to XML
 */
void Caller::calculateProteinProbabilitiesFido() {
  time_t startTime_;
  clock_t startClock_;
  time(&startTime_);
  startClock_ = clock();  

  if (VERB > 0) {
    cerr << "\nCalculating protein level probabilities with Fido\n";
    cerr << protEstimator_->printCopyright();
  }
  
  protEstimator_->initialize(&allScores_);
  protEstimator_->run();
  protEstimator_->computeProbabilities();
  protEstimator_->computeStatistics();
  
  time_t procStart;
  clock_t procStartClock = clock();
  time(&procStart);
  double diff_time = difftime(procStart, startTime_);
  
  if (VERB > 1) {  
    cerr << "Estimating Protein Probabilities took : "
    << ((double)(procStartClock - startClock_)) / (double)CLOCKS_PER_SEC
    << " cpu seconds or " << diff_time << " seconds wall time" << endl;
  }
  
  protEstimator_->printOut(proteinResultFN_, decoyProteinResultFN_);
  if (xmlInterface_.getXmlOutputFN().size() > 0) {
      xmlInterface_.writeXML_Proteins(protEstimator_);
  }
}

/** 
 * Executes the flow of the percolator process:
 * 1. reads in the input file
 * 2. trains the SVM
 * 3. calculate PSM probabilities
 * 4. (optional) calculate peptide probabilities
 * 5. (optional) calculate protein probabilities
 */
int Caller::run() {  

  time(&startTime_);
  startClock_ = clock();
  if (VERB > 0) {
    cerr << extendedGreeter();
  }
  
  // Reading input files (pin or temporary file)
  if (!readFiles()) {
    std::cerr << "ERROR: Failed to read in file, check if the correct " <<
                 "file-format was used.";
    return 0;
  }
  // Copy feature data to Scores object
  fillFeatureSets();
  
  if (VERB > 2) {
    std::cerr << "FeatureNames::getNumFeatures(): "<< FeatureNames::getNumFeatures() << endl;
  }
  int firstNumberOfPositives = crossValidation_.preIterationSetup(allScores_, pCheck_, pNorm_);
  if (VERB > 0) {
    cerr << "Estimating " << firstNumberOfPositives << " over q="
        << testFdr_ << " in initial direction" << endl;
  }
  
  if (tabOutputFN_.length() > 0) {
    setHandler_.writeTab(tabOutputFN_, pCheck_);
  }
  
  time_t procStart;
  clock_t procStartClock = clock();
  time(&procStart);
  double diff = difftime(procStart, startTime_);
  if (VERB > 1) cerr << "Reading in data and feature calculation took "
      << ((double)(procStartClock - startClock_)) / (double)CLOCKS_PER_SEC
      << " cpu seconds or " << diff << " seconds wall time" << endl;
  
  // Do the SVM training
  crossValidation_.train(pNorm_);
  crossValidation_.postIterationProcessing(allScores_, pCheck_);
  // calculate psms level probabilities
  
  //PSM probabilities TDA or TDC
  calculatePSMProb(false, procStart, procStartClock, diff);
  if (xmlInterface_.getXmlOutputFN().size() > 0){
    xmlInterface_.writeXML_PSMs(allScores_);
  }
  
  // calculate unique peptides level probabilities WOTE
  if (reportUniquePeptides_){
    calculatePSMProb(true, procStart, procStartClock, diff);
    if (xmlInterface_.getXmlOutputFN().size() > 0){
      xmlInterface_.writeXML_Peptides(allScores_);
    }
  }
  // calculate protein level probabilities with FIDO
  if(ProteinProbEstimator::getCalcProteinLevelProb()){
    calculateProteinProbabilitiesFido();
  }
  // write output to file
  xmlInterface_.writeXML(allScores_, protEstimator_, call_);  
  return 1;
}
