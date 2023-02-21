FROM ubuntu:18.04

# install prerequisite packages
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install build-essential build-essential automake git libncurses5-dev texinfo expat libexpat1-dev wget curl zsh qemu libx11-dev libxrandr-dev cgdb

WORKDIR /opt
RUN git clone https://github.com/PKU-OS/pintos.git

# build toolchain
ENV SWD=/opt/toolchain
ENV PINTOS_ROOT=/opt/pintos
RUN mkdir -p $SWD 
RUN cd $PINTOS_ROOT && src/misc/toolchain-build.sh $SWD
RUN cd ${PINTOS_ROOT} && src/misc/bochs-2.6.2-build.sh $SWD/x86_64
ENV dest=$SWD/x86_64
RUN cd $PINTOS_ROOT/src/utils &&\
    make &&\
    cp backtrace pintos Pintos.pm pintos-gdb pintos-set-cmdline pintos-mkdisk setitimer-helper squish-pty squish-unix $dest/bin &&\
    mkdir $dest/misc &&\
    cp ../misc/gdb-macros $dest/misc
RUN rm -rf /opt/pintos

RUN chsh -s $(which zsh)
RUN sh -c "$(curl -fsSL https://raw.github.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
RUN zsh -c "git clone https://github.com/zsh-users/zsh-syntax-highlighting.git ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting"
RUN zsh -c "git clone https://github.com/zsh-users/zsh-autosuggestions ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-autosuggestions"
RUN zsh -c "sed -i 's/plugins=(git)/plugins=(git zsh-syntax-highlighting zsh-autosuggestions)/g' ~/.zshrc"
RUN echo PATH=/opt/toolchain/x86_64/bin:$PATH >> ~/.zshrc

CMD ["sleep", "infinity"]