function [p] = decoded_play(fname, fs)

  v = load(fname);
  v = v / max(max(v));
  t = (1:length(v))/fs;

  subplot(2, 1, 1);
  plot(t, v(:,1));
  title(fname);
  xlabel('s');
  ylabel('amplitude');
  axis tigh;
  ylim([-1 1]);

  subplot(2, 1, 2);
  plot(t, v(:,2));
  title(fname);
  xlabel('s');
  ylabel('amplitude');
  axis tight;
  ylim([-1 1]);

  p = audioplayer(v, fs);

end
