echo "Installing Gecode..."
curl -SL "https://github.com/Gecode/gecode/archive/release-6.2.0.tar.gz" | tar xz
mv gecode-release-6.2.0 gecode
cd gecode
./configure --disable-minimodel --disable-examples --disable-flatzinc --disable-gist --disable-driver --disable-qt --disable-mpfr --disable-doc-tagfile --disable-doc-dot
echo "Compiling Gecode"
make -j4
echo "Installing gecode to default prefix"
sudo make install