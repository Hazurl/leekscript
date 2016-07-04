FROM gcc:5.3

MAINTAINER Leek Wars "https://github.com/leek-wars"

# Install required packages
RUN apt-get update
RUN apt-get install -y --force-yes texinfo bison flex

# Install libjit
RUN git clone git://git.savannah.gnu.org/libjit.git libjit
RUN cd libjit; ./bootstrap; ./configure; make; make install
RUN cp /usr/local/lib64/libjit* /usr/lib

# Copy project files
COPY . leekscript