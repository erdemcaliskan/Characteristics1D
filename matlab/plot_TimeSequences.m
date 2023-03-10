function plot_TimeSequences(maxNumInterfaces, dim, ringProb)
if nargin < 1
    root = '../run_0';
end
if nargin < 2
    maxNumInterfaces = 1000;
end
if nargin < 3
    dim = 1;
end
if nargin < 4
    ringProb = 0;
end

if (strcmp(root, '') == 0)
    rt = [root, '/'];
else
    rt = '';
end


labels = {'time', 'sigmaL', 'sigmaR', 'vL',	'vR',  'uL', 'uR', 'delu', ...
    'pL', 'pR', 'eL', 'del_e', 'diss', ...,
    'damage', 'damageSource', 'maxEffdelu', 'v_r', 'v_r_source', 'sigma_theta_source'};
ln = length(labels);
if (ringProb == 0)
    ln = ln - 3;
end


i = 0;
for ii = 1:maxNumInterfaces
%   if (ii ~= 4)
%       continue;
%   end
    fileName = [rt, 'xPos_', num2str(ii - 1), '_InterfaceRawFinalSln.txt'];
    fid = fopen(fileName, 'r');
    if (fid > 0)
        fclose(fid);
        i = i + 1;
        [data{i}, delu{i}, delv{i}, sigmaL{i}, sigmaR{i}, diss{i}, del_e{i}, exists] = loadTimeSequence(fileName, dim, ringProb);
    end
end
maxNumInterfaces = i; % - 1;




if 0 % to see the time history
    for n = 1:20:2000 %5000
        d = 1;
        st = 1;
        for i = 1:maxNumInterfaces
            figure(1000 + i);
             times = data{i}{1};
             deluv =delu{i}(:, d);
             sigmav = sigmaR{i}(:, d);
             if (n > 1)
                 times = times(st:n);
                 deluv = deluv(st:n);
                 sigmav = sigmav(1:n);
             end
             plot(deluv, sigmav);
             title(['t = ', num2str(times(n))]);
             print('-dpng', ['delusigma', num2str(n), '.png']);
        end
    end
end

dataOut = data{1}; 
cntr = 0;
%    times = dataOut{1};
for j = 2:ln
    dat = dataOut{j};
    [m, n] = size(dat);
    szz(j) = n;
    indices{j} = cntr + 1:cntr + n;
    for k = 1:n
        cntr = cntr + 1;
        labs{cntr} = [labels{j}, num2str(k)];
    end
end

if (maxNumInterfaces < 20)
    lbnum = 0:maxNumInterfaces - 1;
    lbstr = cell(1, maxNumInterfaces);
    for i = 1:maxNumInterfaces
        num2str(lbnum(i), lbstr{i});
    end
end

figure(1);
clf
for i = 1:maxNumInterfaces
   du = delu{i};
   sg = sigmaR{i};
   plot(du, sg);
   leg{i} = num2str(i - 1);
   hold on;
end
lg = legend(leg, 'FontSize', 9);
legend('boxoff');
plotName = [rt, 'plot_delu_sigma'];
print('-dpng', [plotName, '.png']);
savefig([plotName, '.fig']);
clf

for j = 2:ln
    n = szz(j);
    for k = 1:n
        cntr = indices{j}(k);
        figure(cntr);
        for i = 1:maxNumInterfaces
            dataOut = data{i};
            times = dataOut{1};
            dat = dataOut{j};
            y = dat(:, k);
            plot(times, y);
            hold on;
        end
        plotName = [rt, 'plot', labs{cntr}];
        print('-dpng', [plotName, '.png']);
        savefig([plotName, '.fig']);
    end
end

% cntr = 0;
% for j = 2:ln
%     dat = dataOut{j};
%     [m, n] = size(dat);
%     for k = 1:n
%         cntr = cntr + 1;
%         plotName = ['plot', labs{cntr}];
%         print('-dpng', [plotName, '.png']);
%         savefig([plotName, '.fig']);
%     end
% end



if 0
    furthrChck = zeros(1, maxNumInterfaces);
    d = 1;
    if 1
    for i = 1:maxNumInterfaces
        times = data{i}{1};
        deluv =delu{i}(:, d);
        delvv =delv{i}(:, d);
        sigmav = sigmaR{i}(:, d);
        dissv = diss{i}(:);
        del_ev = del_e{i}(:);
        figure(1000 + i);
        plot(deluv, sigmav);
        title(['int = ', num2str(i - 1)]);
        plotName = [rt, 'plot_delu_sigma_interface_', num2str(i - 1)];
        print('-dpng', [plotName, '.png']);
        savefig([plotName, '.fig']);
        print('-dpng', [plotName, '.png']);

        clf;
        plot(times, dissv);
        plotName = [rt, 'plot_time_diss_interface_', num2str(i - 1)];
        print('-dpng', [plotName, '.png']);
        savefig([plotName, '.fig']);
        print('-dpng', [plotName, '.png']);

        clf;
        plot(times, del_ev);
        plotName = [rt, 'plot_time_del_ev_interface_', num2str(i - 1)];
        print('-dpng', [plotName, '.png']);
        savefig([plotName, '.fig']);
        print('-dpng', [plotName, '.png']);

        clf;
        plot(times, deluv);
        plotName = [rt, 'plot_time_delu_interface_', num2str(i - 1)];
        print('-dpng', [plotName, '.png']);
        savefig([plotName, '.fig']);
        print('-dpng', [plotName, '.png']);
        
        clf;
        plot(times, delvv);
        plotName = [rt, 'plot_time_delv_interface_', num2str(i - 1)];
        print('-dpng', [plotName, '.png']);
        savefig([plotName, '.fig']);
        print('-dpng', [plotName, '.png']);
        
       clf;
        plot(times, sigmav);
        plotName = [rt, 'plot_time_sigma_interface_', num2str(i - 1)];
        print('-dpng', [plotName, '.png']);
        savefig([plotName, '.fig']);
        print('-dpng', [plotName, '.png']);
        
        mn = min(dissv);
        mx = max(dissv);
        if (mn < -1e-3 * mx)
            furthrChck(i) = 1;
        end
        mn = min(del_ev);
        mx = max(del_ev);
        if (mn < -1e-3 * mx)
            furthrChck(i) = 1;
        end
    end
    end
%    furthrChck = 0 * furthrChck;
%furthrChck(2 + 1) = 1;
%furthrChck = 0 * furthrChck;
%furthrChck(15) = 1;
    d = 1;
    for i = 1:maxNumInterfaces
        if (furthrChck(i) == 0)
            continue;
        end

        timesALL = data{i}{1};
        deluvAll =delu{i}(:, d);
        delvvAll =delv{i}(:, d);
        sigmavAll = sigmaR{i}(:, d);
        dissvAll = diss{i}(:);
        del_evAll = del_e{i}(:);

        sz = length(deluvAll);
        stp = max(1, floor(sz / 10));

        for j = 1:stp:sz
%        for j = 3000:10:3200
            times = timesALL(1:j);
            tm = times(j);
            deluv = deluvAll(1:j);
            delvv = delvvAll(1:j);
            sigmav = sigmavAll(1:j);
            dissv = dissvAll (1:j);
            del_ev = del_evAll(1:j);

            figure(1);
            clf;
            plot(deluv, sigmav);
            hold on;
            plot(deluv(j), sigmav(j), '-xr', 'MarkerSize', 6);
            title(['t = ', num2str(tm), ' int = ', num2str(i - 1)]);
            plotName = [rt, 'plot_delu_sigma_interface_', num2str(i - 1), '_j_', num2str(j - 1)];
            print('-dpng', [plotName, '.png']);
            savefig([plotName, '.fig']);
            print('-dpng', [plotName, '.png']);


            clf;
            plot(times, deluv);
            hold on;
            plot(times(j), deluv(j), '-xr', 'MarkerSize', 6);
            title(['t = ', num2str(tm), ' int = ', num2str(i - 1)]);
            plotName = [rt, 'plot_time_delu_interface_', num2str(i - 1), '_j_', num2str(j - 1)];
            print('-dpng', [plotName, '.png']);
            savefig([plotName, '.fig']);
            print('-dpng', [plotName, '.png']);
            

            clf;
            plot(times, sigmav);
            hold on;
            plot(times(j), sigmav(j), '-xr', 'MarkerSize', 6);
            title(['t = ', num2str(tm), ' int = ', num2str(i - 1)]);
            plotName = [rt, 'plot_time_sigmav_interface_', num2str(i - 1), '_j_', num2str(j - 1)];
            print('-dpng', [plotName, '.png']);
            savefig([plotName, '.fig']);
            print('-dpng', [plotName, '.png']);
            
            clf;
            plot(times, delvv);
            hold on;
            plot(times(j), delvv(j), '-xr', 'MarkerSize', 6);
            title(['t = ', num2str(tm), ' int = ', num2str(i - 1)]);
            plotName = [rt, 'plot_time_delv_interface_', num2str(i - 1), '_j_', num2str(j - 1)];
            print('-dpng', [plotName, '.png']);
            savefig([plotName, '.fig']);
            print('-dpng', [plotName, '.png']);
            
            clf;
            plot(times, dissv);
            hold on;
            plot(times(j), dissv(j), '-xr', 'MarkerSize', 6);
            title(['t = ', num2str(tm), ' int = ', num2str(i - 1)]);
            plotName = [rt, 'plot_time_diss_interface_', num2str(i - 1), '_j_', num2str(j - 1)];
            print('-dpng', [plotName, '.png']);
            savefig([plotName, '.fig']);
            print('-dpng', [plotName, '.png']);

            clf;
            plot(times, del_ev);
            hold on;
            plot(times(j), del_ev(j), '-xr', 'MarkerSize', 6);
            title(['t = ', num2str(tm), ' int = ', num2str(i - 1)]);
            plotName = [rt, 'plot_time_del_e_interface_', num2str(i - 1), '_j_', num2str(j - 1)];
            print('-dpng', [plotName, '.png']);
            savefig([plotName, '.fig']);
            print('-dpng', [plotName, '.png']);
        end
    end
end

fclose('all');
close('all');