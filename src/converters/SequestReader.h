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

#ifndef SEQUESTREADER_H
#define SEQUESTREADER_H

#include "MzidentmlReader.h"
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

class SequestReader : public MzidentmlReader {

  public:

    SequestReader(ParseOptions po);
    virtual ~SequestReader();
    bool checkValidity(const std::string &file);
    void addFeatureDescriptions(bool doEnzyme);
    void createPSM(const ::mzIdentML_ns::SpectrumIdentificationItemType & item,
		  ::percolatorInNs::fragSpectrumScan::experimentalMass_type experimentalMass,
		   bool isDecoy, unsigned useScanNumber, boost::shared_ptr<FragSpectrumScanDatabase> database,
		   const std::string & fn);

   protected :

    static const std::map<string,int> sequestFeatures;
};

#endif // SEQUESTREADER_H
