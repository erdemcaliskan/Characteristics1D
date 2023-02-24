Zl = 3.20406;
Zr = 3.20406;
%Zr = 1.55563;
%Zl = 1.55563;
%Zr = 16;
%Zl = 16;
ZsumInv = 1.0 / (Zl + Zr);
YAve_n = 0.5 * (1.0/Zl + 1.0/Zr);
iso_wlSide_rGoing_2_sigmaI = Zr * ZsumInv;
iso_wrSide_lGoing_2_sigmaI = Zl * ZsumInv;
iso_wlSide_rGoing_2_vI = -ZsumInv;
iso_wrSide_lGoing_2_vI = ZsumInv;
iso_sigmaStar_2_vStar_lSide = 1.0 / Zl;
iso_sigmaStar_2_vStar_rSide = -1.0 / Zr;
iso_wlSide_rGoing_2_vStar_lSide = -1.0 / Zl;
iso_wrSide_lGoing_2_vStar_rSide = 1.0 / Zr;
fprintf(1, 'YAve_n\t%g\n', YAve_n);
fprintf(1, 'iso_wlSide_rGoing_2_sigmaI\t%g\n', iso_wlSide_rGoing_2_sigmaI);
fprintf(1, 'iso_wrSide_lGoing_2_sigmaI\t%g\n', iso_wrSide_lGoing_2_sigmaI);
fprintf(1, 'iso_wlSide_rGoing_2_vI\t%g\n', iso_wlSide_rGoing_2_vI);
fprintf(1, 'iso_wrSide_lGoing_2_vI\t%g\n', iso_wrSide_lGoing_2_vI);
fprintf(1, 'iso_sigmaStar_2_vStar_lSide\t%g\n', iso_sigmaStar_2_vStar_lSide);
fprintf(1, 'iso_sigmaStar_2_vStar_rSide\t%g\n', iso_sigmaStar_2_vStar_rSide);
fprintf(1, 'iso_wlSide_rGoing_2_vStar_lSide\t%g\n', iso_wlSide_rGoing_2_vStar_lSide);
fprintf(1, 'iso_wrSide_lGoing_2_vStar_rSide\t%g\n', iso_wrSide_lGoing_2_vStar_rSide);
