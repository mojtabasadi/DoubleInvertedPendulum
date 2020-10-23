clc
clear all
close all
%Initialize COM PORT by 57600bit/Sec BaudRate
s = serialport("COM3",57600);
%Please Set First Position to 0 after Start
Position=0;
Angle1=0;
%Set First Desire Position and Speed
%DesirePosition Range Value From 100 as Minimum to 10000 as Maximum
DesirePosition=500;
%'W' equal to Weak Speed
%'L' equal to Low Speed
%'M' equal to Medium Speed
%'H' equal to High Speed
%'V' equal to Very High Speed
Speed='H';
%Infinite Control Loop
while(1)
    %Write Your Control Algorithm HERE
    
    if (DesirePosition>10000)
        DesirePosition=10000;
    end
    if (DesirePosition<150)
        DesirePosition=150;
    end

    %Send Desire Position and Desire Speed Data to Interface Board
    write(s,"PC","string");
    write(s,DesirePosition,"uint16");
    write(s,Speed,"string");
    CheckSum=147+fix(mod(DesirePosition,256))+fix(DesirePosition/256)+Speed;
    write(s,fix(bitand((CheckSum),255)),"uint8");
    write(s,13,"uint8");
    write(s,10,"uint8");
    %Give FeedBack Data From Interface Board
    FeedBack(1)=read(s,1,"uint8");
    if FeedBack(1) == 'I'
        FeedBack(2)=read(s,1,"uint8");
        if FeedBack(2) == 'P'
           for i=3:11
               FeedBack(i)=read(s,1,"uint8");              
           end
           CheckSum=0;
           for i=1:8
               CheckSum=CheckSum+FeedBack(i);
               if fix(bitand(CheckSum,255)) == FeedBack(9)
                    Position = FeedBack(3) + bitshift(FeedBack(4),8);
                    Angle1   = FeedBack(5) + bitshift(FeedBack(6),8);
                    Angle1   = Angle1*0.3;
                    Angle2   = FeedBack(7) + bitshift(FeedBack(8),8);
                    Angle2   = Angle2*7.5;
               end
           end
        end
    end
end
