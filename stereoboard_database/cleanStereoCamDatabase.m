clear all
close all


%% Clean Images
%Load Images (uses sort_nat)
for track = 1:4
    disp(track)
    dirname=['database_stereoboard_2/downward_camera/take',num2str(track),'/'];
    srcFiles = dir([dirname,'*.bmp']);
    names= {srcFiles.name}';
    names= sort_nat(names);
    
    for i= 1: size(names,1)
        filename = strcat(dirname,names{i});
        I=imread(filename);
%         I=rgb2gray(imread(filename));
                I=I(1:96,1:256);
%         I=[I;zeros(2,256)];
        imwrite(I,filename)
    end
    
end