%{ 
Note: Projekt byl vypracovan s vyuzitim programu
      GNU Octave. Ten ma vetsinu syntaxe shodnou
      s Matlabem, ale mohou se vyskytnout vyrazy,
      ktere Matlab nezpracuje.
Octave necessary commands:
      pkg load image
%}

%% Zaostreni obrazu
I = imread('xsumsa01.bmp');
H = [ -0.5 -0.5 -0.5; -0.5 5.0 -0.5; -0.5 -0.5 -0.5 ]
I2 = imfilter(I, H);
% imshow(I2);
imwrite(I2, 'step1.bmp');

%% Otoceni obrazu
I = imread('step1.bmp');
I2 = flipdim(I, 2);
% imshow(I2);
imwrite(I2, 'step2.bmp');

%% Medianovy filtr
I = imread('step2.bmp');
I2 = medfilt2(I, [5 5]);
% imshow(I2);
imwrite(I2, 'step3.bmp');

%% Rozmazani obrazu
I = imread('step3.bmp');
H = [ 1 1 1 1 1; 1 3 3 3 1; 1 3 9 3 1; 1 3 3 3 1; 1 1 1 1 1 ] / 49
I2 = imfilter(I, H);
% imshow(I2);
imwrite(I2, 'step4.bmp');

%% Chyba v obraze
Io = imread('xsumsa01.bmp');
Im = imread('step4.bmp');
Im = flipdim(Im, 2);
Io = double(Io);
Im = double(Im);
noise = 0

for(x = 1:512)
    for(y = 1:512)
        noise = noise + double(abs(Io(x, y) - Im(x,y)));
    end;
end;
noise = noise / (512^2)

%% Roztazeni histogramu
I = imread('step4.bmp');
% imhist(I);
I = double(I);
[Imin, Iminx] = min(I(:));
[Imax, Imaxx] = max(I(:));

for(x = 1:512)
    for(y = 1:512)
        I(x, y) = ((I(x, y) - Imin) / (Imax - Imin));
    end;
end;
% imhist(I);
% I = uint8(I);
imwrite(I, 'step5.bmp');

%% Spocitani stredni hodnoty a smerodatne odchylky
Inohist = imread('step4.bmp');
Ihist = imread('step5.bmp');
Inohist = double(Inohist);
Ihist = double(Ihist);
mean_no_hist = mean(mean(Inohist))
std_no_hist = std2(Inohist)
mean_hist = mean(mean(Ihist))
std_hist = std2(Ihist)

%% Kvantizace obrazu
I = imread('step5.bmp');
[Im, map] = gray2ind(I, 4);
I = ind2gray(Im, map);
% imshow(I);
imwrite(I, 'step6.bmp');
