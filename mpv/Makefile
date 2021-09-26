CC      = gcc 
NOCRYPT =
1BUILD  = 
C_FLAGS = $(1BUILD) -g -mtune=athlon64 -fno-builtin-pow -Wno-unused -DLinux -DNEED_EXTERN -DOLD_FIGHT $(PROF) $(NOCRYPT)
L_FLAGS = $(PROF) -lcrypt

O_FILES =  autoeq.o aedit.o alien.o ansi_out.o ar.o arena.o armor.o aw.o \
	auction.o bank.o boat.o buysell.o chan.o channels.o char.o clan.o climb.o \
	cmd.o config.o conflict.o consid.o comm.o convo.o crsh.o dam.o dam2.o \
	db2.o death.o def.o dict.o draw.o edit.o fgt.o give.o grnd.o guild.o \
	help.o hnd.o inetd.o int.o info.o look.o medit.o menu.o misc.o mgc.o move.o \
	note.o object.o oc.o oedit.o pb.o pedit.o pets.o pinfo.o power.o prompt.o \
	prse.o quit.o redit.o reloc.o remort.o rfd.o room.o score.o script3.o \
	script.o script2.o sedit.o spec.o spl.o skill.o sv.o syslag.o toptr.o \
	upd.o view.o warmth.o yell.o combat.o pc_wiz.o shell.o hedit.o

# poll.o removed just to compile rest of code. Add it back in and fix.
# Owen crap:
#    mob_1514.o mob_2013.o mob_1114.o mob_101.o mob_1029.o

EmlenMud: $(O_FILES) 
	rm -f core MpV
	ctags -w *.c *.h&
	# [1mLinking...[0m
	$(CC) $(L_FLAGS) -o MpV $(O_FILES)
	# [1mDONE![0m

.c.o: emlen.h
	$(CC) -c $(C_FLAGS) $<

install: 
	cp -f MpV /home/mud/demonspawn-mpv3/kilith/svn/vor/src/MpV.patch
	# [1mOk now all you have to do is reboot the mud![0m
patch:
	cp -f MpV /home/mud/demonspawn-mpv3/kilith/mud/src/MpV.patch
	# [1mOk now all you have to do is reboot the mud![0m
clean:
	# [1mCleaning up...[0m
	rm -f *.o core MpV
7777: 
	# [1mPatching player port (7777)...[0m
	cp -f MpV ../src/MpV.patch
	# [1mOk now all you have to do is reboot the mud![0m
4445: 
	# [1mPatching builder port (4445)...[0m
	cp -f EmlenMud /home/mud/demonspawn-mpv3/kilith/mud/src/MpV.patch
	# [1mOk now all you have to do is reboot the mud![0m
