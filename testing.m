clear all
close all
clc

% load camera data
cam_file = fopen('stereoboard_database/Take16/result.csv');
% cam_file = fopen('stereoboard_database/Track3/result.csv');
cam = textscan(cam_file,'%f,%f,%f,%f,%f,%f,%f,%f,%f');
fclose(cam_file);

x_pixelwise_kirk = cam{1}/100.;
z_pixelwise_kirk = cam{2}/100.;
x_global_kirk = cam{3}/100.;
y_global_kirk = cam{4}/100.;
z_global_kirk = cam{5}/100.;

time = 0:1/8:numel(x_pixelwise_kirk)/8 - 1/8;
% cam_file = fopen('/home/kirk/mavlab/stereoboard/stereoboard_testing/stereoboard_database/Track3/result_kim.csv');
% cam = textscan(cam_file,'%f,%f,%f,%f,%f,%f,%f,%f,%f');
% fclose(cam_file);
% 
% x_pixelwise_kim = cam{1}/100;
% z_pixelwise_kim = cam{2}/100;
% x_global_kim = cam{3}/100;
% y_global_kim = cam{4}/100;
% z_global_kim = cam{5}/100;

% figure(2)
% subplot(3,1,1)
% plot([x_pixelwise_kirk, x_global_kirk, x_global_kim]); ylim([-1,1])
% subplot(3,1,2)
% plot(y_global_kirk); ylim([-1,1])
% subplot(3,1,3)
% plot([z_pixelwise_kirk, z_global_kirk]); ylim([-1,1])



%%
% [cam_Vx_frame, cam_Vz_frame, yaw_frame, t_frame] = getOptiTrack('/home/kirk/mavlab/stereoboard/ext/stereoboard_testing/stereoboard_database/Track3');
[cam_Vx_frame, cam_Vz_frame, yaw_frame, t_frame] = getOptiTrack(16,'stereoboard_database/Take');


figure,
subplot(3,1,1)
plot(t_frame(1:end-1),[x_pixelwise_kirk, x_global_kirk]); ylim([-1,1])
hold on, plot(t_frame,-cam_Vx_frame);
subplot(3,1,2)
plot(t_frame(1:end-1), y_global_kirk);% ylim([-1,1])
subplot(3,1,3)
plot(t_frame(1:end-1),[z_pixelwise_kirk, z_global_kirk]); ylim([-1,1])
hold on, plot(t_frame,-cam_Vz_frame);
