clear all
close all
track = 13;


%% Clean Images 
%Load Images (uses sort_nat)
dirname=['D:\knmcguire\My Documents\OneDrive\PhD\Experiments\DataBase Stereoboard New\take',num2str(track),'\'];
srcFiles = dir([dirname,'*.bmp']);
names= {srcFiles.name}';
names= sort_nat(names);

for i= 1: size(names,1)
       filename = strcat(dirname,names{i});
    I=imread(filename);
        I=I(1:94,1:256);
    imwrite(I,filename)
end

