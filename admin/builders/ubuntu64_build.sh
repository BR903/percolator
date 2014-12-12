#!/bin/bash
# managing input arguments

while getopts “s:b:r:t:” OPTION; do
  case $OPTION in
    s) src_dir=${OPTARG};;
    t) branch=${OPTARG};;
    r) release_dir=${OPTARG};;
    b) build_dir=${OPTARG};;
    \?) echo "Invalid option: -${OPTARG}" >&2;;
  esac
done

if [[ -z ${build_dir} ]]; then
  build_dir="$(mktemp -d --tmpdir ubuntu_build_XXXX)";
fi
if [[ -z ${src_dir} ]]; then
  if [[ -n  ${branch} ]]; then
    sudo apt-get install git;
    src_dir="$(mktemp -d --tmpdir ubuntu_build_XXXX)";
    git clone --branch "$1" https://github.com/percolator/percolator.git "${src_dir}/percolator";
  else
    src_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/../../../
  fi
fi
if [[ -z ${release_dir} ]]; then
  release_dir=${HOME}/release
  if [ ! -d "${release_dir}" ]; then
    mkdir ${release_dir}
  fi
fi

echo "The Builder $0 is building the Percolator packages with src=${src_dir} an\
d build=${build_dir} for the user"
whoami;

#------------------------------------------------------------------------
#------------------------------------------------------------------------
echo "Checking necessary packages for building percolator...";

# Do not apt-update if this is a travis-ci job
if [ -z "$TRAVIS" ]; then
  sudo apt-get update;
  sudo apt-get upgrade;
  sudo apt-get -y install g++ make cmake rpm fakeroot;
fi

mkdir -p $build_dir
cd ${build_dir}

# download and patch xsd
xsd=xsd-3.3.0-x86_64-linux-gnu
if [ ! -d ${xsd} ]; then
  wget --quiet http://www.codesynthesis.com/download/xsd/3.3/linux-gnu/x86_64/${xsd}.tar.bz2
  tar xjf ${xsd}.tar.bz2
  sed -i 's/setg/this->setg/g' ${xsd}/libxsd/xsd/cxx/zc-istream.txx
  sed -i 's/ push_back/ this->push_back/g' ${xsd}/libxsd/xsd/cxx/tree/parsing.txx
  sed -i 's/ push_back/ this->push_back/g' ${xsd}/libxsd/xsd/cxx/tree/stream-extraction.hxx
fi

# end of section to remove
sudo apt-get -y install libxerces-c-dev libboost-dev libboost-filesystem-dev;
sudo apt-get -y install libboost-system-dev libboost-thread-dev libsqlite3-dev libtokyocabinet-dev zlib1g-dev libbz2-dev;

#------------------------------------------------------------------------
mkdir -p $build_dir/percolator-noxml $build_dir/percolator $build_dir/converters $build_dir/elude;

######percolator########
#-----cmake-----
cd $build_dir/percolator-noxml;
echo -n "cmake percolator.....";
cmake -DTARGET_ARCH=amd64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DXML_SUPPORT=OFF $src_dir/percolator;
#-----make------
echo -n "make percolator (this will take few minutes).....";
make -j 4;
make -j 4 package;

#-----cmake-----
cd $build_dir/percolator;
echo -n "cmake percolator.....";
cmake -DTARGET_ARCH=amd64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DXML_SUPPORT=ON $src_dir/percolator -DCMAKE_PREFIX_PATH="${build_dir}/${xsd}/";
#-----make------
echo -n "make percolator (this will take few minutes).....";
make -j 4;
make -j 4 package;

#######converters########
cd $build_dir/converters
#-----cmake-----
echo -n "cmake converters.....";
cmake -DTARGET_ARCH=amd64 -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DSERIALIZE="TokyoCabinet" $src_dir/percolator/src/converters -DCMAKE_PREFIX_PATH="${build_dir}/${xsd}/";

#-----make------
echo -n "make converters (this will take few minutes).....";

make -j 4;
make -j 4 package;

#######elude########
cd $build_dir/elude
#-----cmake-----
echo -n "cmake elude.....";
cmake -DTARGET_ARCH=amd64 -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release $src_dir/percolator/src/elude_tool;

#-----make------
echo -n "make elude (this will take few minutes).....";

make -j 4;
make -j 4 package;

###########################
cp $build_dir/{percolator-noxml,percolator,converters,elude}/*.deb ${release_dir};
echo "Finished buildscript execution";
echo "in build directory ${build_dir}";
