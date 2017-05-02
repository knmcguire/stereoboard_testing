%function testing_stereoboard(stereoboard_type, configuration, take_nr)
% for example testing_stereoboard(1,'forward_camera', 16)


% load camera data
dir = ['stereoboard_database/database_stereoboard_',num2str(stereoboard_type),'/',configuration,'/take',num2str(take_nr)];
cam_file = fopen([dir,'/result_stereo.csv']);
cam = textscan(cam_file,'%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f');
fclose(cam_file);

x_pixelwise_stereo = cam{1}/100.;
z_pixelwise_stereo = cam{2}/100.;
x_global_stereo = cam{3}/100.;
y_global_stereo = cam{4}/100.;
z_global_stereo = cam{5}/100.;

%%
[cam_Vx_frame, cam_Vz_frame, yaw_frame, t_frame] = getOptiTrack(dir);


figure,
subplot(3,1,1)
plot(t_frame(1:end-1),[x_pixelwise_stereo, x_global_stereo]); ylim([-1,1])
hold on, plot(t_frame,-cam_Vx_frame);
subplot(3,1,2)
plot(t_frame(1:end-1), y_global_stereo); ylim([-1,1])
subplot(3,1,3)
plot(t_frame(1:end-1),[z_pixelwise_stereo, z_global_stereo]); ylim([-1,1])
hold on, plot(t_frame,-cam_Vz_frame);
