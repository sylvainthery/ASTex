#ifndef COLOR_MAP_H
#define COLOR_MAP_H

#include <ASTex/image_rgb.h>
#include <map>
#include <ASTex/utils.h>
#include <ASTex/easy_io.h>

namespace ASTex {


template <typename T>
class Color_map
{
public :
    using Color = Eigen::Matrix<T,3,1>;
    using Vec2 = Eigen::Matrix<T,2,1>;
private:
    std::map<int, Color > palette;
    T step = T(0);

    ImageRGB<T> filtered;
    T sigma_max;
    int width, height;
    bool isfiltered = false;

    Color map(const T &x) const {
        Color ret;
        if(x < T(0))
            ret = palette.begin()->second;
        else if ( x > T(1))
            ret = palette.rbegin()->second;
        else {
            for(auto it = palette.begin();it!=palette.end();++it){
                T inf = T(it->first) * step;
                T sup = T((++it)->first) * step;
                T t = (x - inf) / (sup -inf);
                it--;
                if (x >= inf && x <= sup) {
                    Color c_inf = it->second;
                    Color c_sup = (++it)->second;
                    it--;
                    ret = (1 - t) * c_inf + t * c_sup;
                    break;
                }
            }
        }
        return ret;
    }

    T gauss1D(const T &x, const T &mu, const T &sigma) const
    {
        const T Pi = 4 * std::atan(T(1));

        T ret = std::exp(-T(0.5) * std::pow((x-mu)/sigma,2));
        ret /= sigma * std::sqrt(2 * Pi);

        return ret;
    }


//    T numeric_integration_gauss1D(const T&a, const T&b, const int &n, const T&mu, const T& sigma){
//        T sum(0);
//        for(int k= 1; k <= n-1; ++k){
//            T v =a + T(k) * (b - a) / T(n);
//            sum += gauss1D(v, mu, sigma);
//        }
//        sum += (gauss1D(a, mu, sigma) + gauss1D(b, mu, sigma)) * T(0.5);
//        sum *= (b-a) / T(n);

//        return sum;
//    }

//    Color numeric_integration_col_gauss1D(const T&a, const T&b, const int &n, const T&mu, const T& sigma){
//        Color sum(0,0,0);

//        for(int k= 1; k <= n-1; ++k){
//            T v = a + T(k) * (b - a) / T(n);
//            sum += map(v) * gauss1D(v, mu, sigma);
//        }
//        sum += (map(a) * gauss1D(a, mu, sigma) + map(b) * gauss1D(b, mu, sigma)) * T(0.5);
//        sum *= (b-a) / T(n);

//        return sum;
//    }

    template<typename T2, typename init, typename func>
    inline T2 numeric_integration_1D(const T&a, const T&b, const int &n,const init &initialisation,const func &f){
        T2 sum = initialisation();
        for(int k= 1; k <= n-1; ++k){
            T v =a + T(k) * (b - a) / T(n);
            sum += f(v);
        }
        sum += (f(a) + f(b)) * T(0.5);
        sum *= (b-a) / T(n);

        return sum;
    }

    T numeric_integration_gauss1D(const T&a,
                                  const T&b,
                                  const int &n,
                                  const T&mu,
                                  const T& sigma)
    {
        return numeric_integration_1D<T>(a, b, n, [](){return T(0);}, [&](const T &x){
            return gauss1D(x, mu, sigma);
        });
    }

    Color numeric_integration_col_gauss1D(const T&a,
                                          const T&b,
                                          const int &n,
                                          const T& mu,
                                          const T& sigma)
    {
        return numeric_integration_1D<Color>(a, b, n, [](){return Color(0,0,0);}, [&](const T &x){
            Color ret = map(x) * gauss1D(x, mu, sigma);
            return ret;
        });
    }

public:
    Color_map() {}
    void add_color(const int &pos,const Color &col) {
        palette.insert(std::pair<int, Color>(pos,col));
        step = T(1) / T(palette.rbegin()->first);
    }

    ImageRGB<T> get_filtered() const {
        return filtered;
    }

    void set_filtered(const ImageRGB<T> &i,const T &sm) {
        height = i.height();
        width = i.width();
        filtered = i;
        sigma_max = sm;
    }

    Color map(const T &f, const T &sigma) const {
        int y = f * (height-1);
        int x = sigma * (width-1) / sigma_max;

        x = static_cast<int>(clamp_scalar(T(x), T(0), T(width-1)));
        y = static_cast<int>(clamp_scalar(T(y), T(0), T(height-1)));

        return filtered.pixelEigenAbsolute(x,y);
    }

    void filter(const int &w, const int &h,const int &nb_bins, const T &sm){
        width = w;
        height = h;
        sigma_max = sm;

        filtered = ImageRGB<T>(width,height);
        filtered.parallel_for_all_pixels([&](typename ImageRGB<T>::PixelType &p,int i,int j)
        {
            T f = T(j) / T(h);
            T s = T(i) / T(w) * sigma_max;
            Color c;

            if(s != 0)
                c = numeric_integration_col_gauss1D(-3*s+f,3*s+f,nb_bins,f,s);
            else
                c = map(f);

            p = ImageRGB<T>::itkPixel(c);

        });

        isfiltered = true;

    }

    std::string to_string() const {
        std::stringstream s;
        auto it = palette.begin();
        s << "(" << it->first << " ";
        s << it->second(0) << " " << it->second(1) << " " << it->second(2);
        it++;
        for (;it!=palette.end();++it) {
            s << ", " << it->first << " ";
            s << it->second(0) << " " << it->second(1) << " " << it->second(2);
        }

        s << ")";

        return s.str();
    }

    void export_palette(const std::string &filename) const {
        std::ofstream fd;
        fd.open(filename);
        fd << "set palette defined ";
        fd << to_string();

        fd.close();
    }

    void export_img_palette(const int & h, const std::string &filename) const {
        ImageRGB<T> im(25, h);
        im.parallel_for_all_pixels([&](typename ImageRGB<T>::PixelType &pix, int , int j){
            T y = T(j) / T(im.height());
            if (!isfiltered)
                pix = ImageRGB<T>::itkPixel(map(y));
            else
                pix = ImageRGB<T>::itkPixel(map(y,0));
        });
        IO::save01_in_u8(im, filename);

    }

    void export_courbe(const std::string &filename = "data.txt") const {
        std::ofstream fd;
        fd.open(filename);
        for(auto it = palette.begin();it!= palette.end();it++){
            fd << it->second[0] << " " << it->second[1] << " " << it->second[2] << std::endl;
        }
        fd << std::endl << std::endl;
        int n = 100;
        for (int i =0; i <= n; i++) {
            T x( T(i) / T(n));
            Color c = map(x);
            fd << c[0] << " " << c[1] << " " << c[2] << std::endl;
        }

        fd.close();
    }

    void export_filtered_courbe(const std::string &filename = "data2.txt") const {
        std::ofstream fd;
        fd.open(filename);
        for(auto it = palette.begin();it!= palette.end();it++){
            fd << it->second[0] << " " << it->second[1] << " " << it->second[2] << std::endl;
        }
        fd << std::endl << std::endl;
        int n = 100;
        for (int i =0; i <= n; i++) {
            T y( T(i) / T(n));
                for (int j =0;j <=n;j++) {
                T x( T(j) / T(n));
                Color c = map(x,y);
                fd << c[0] << " " << c[1] << " " << c[2] << std::endl;
            }
        }

        fd.close();

    }
};

}

#endif // COLOR_MAP_H
