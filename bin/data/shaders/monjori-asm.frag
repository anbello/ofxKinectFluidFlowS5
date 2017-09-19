void main()
{
  x.x/=1024;       // convert to 0.0..1.0
  x.y/=768;
  a*=40;
  float d,e,f,g=1.0/40,h,i,p,q; // 1.0 is used instead of 1 to avoid an integer division
  e=400*x.x;		// set effect resolution
  f=400*x.y;		// ...
  i=200+sin(e*g+a/150)*20;
  d=200+cos(f*g/2)*18+cos(e*g)*7; // distort centre
  p=sqrt(pow(i-e,2)+pow(d-f,2));
  q=f/p; 
  e=(p*cos(q))-a/2;
  f=(p*sin(q))-a/2;
  d=sin(e*g)*176+sin(e*g)*164+p;
  h=((f+d)+a/2)*g;
  i=cos(h+p*x.x/1.3)*(e+e+a)+cos(q*g*6)*(p+h/3); 
  h=sin(f*g)*144-sin(e*g)*212*x.x;
  h=(h+(f-e)*q+sin(p-(a+h)/7)*10+i/4)*g;
  i+=cos(h*2.3*sin(a/350-q))*184*sin(q-(p*4.3+a/12)*g)+tan(p*g+h)*184*cos(p*g+h);
  i=fmod(i/5.6,256)/64;
  if(i<0)i+=4;
  if(i>=2)i=4-i;
  d=p/350;
  d+=sin(d*d*8)*0.52;
  f=(sin(a*g)+1)/2;
  return float4(f*i/1.6,i/2+d/13,i,1)*d*x.x+float4(i/1.3+d/8,i/2+d/18,i,1)*d*(1-x.x);
}
