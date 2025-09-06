#ifndef LSMATERIAL_H_
#define LSMATERIAL_H_

#include <iostream>
#include <string>


using namespace std;

/*!
 * \brief The lsMaterial class
 */
class lsMaterial
{
public:
    lsMaterial();
    virtual ~lsMaterial();

    void setDiffuse( float arg1, float arg2, float arg3 );
    void setSpecular( float arg1, float arg2, float arg3 );
    void setAmbient( float arg1, float arg2, float arg3 );
    void setShininess( float arg1 );
    void setGlow( float arg1 );

    float getTransparent() const;
    void setTransparent(float value);
    float getMshininess() const;
    void setMshininess(float value);
    float getMglow() const;
    void setMglow(float value);

    // Get set
    lsMaterial *getNext_material() const;
    void setNext_material(lsMaterial *value);
    string getName() const;
    void setName(const string &value);
    // textures
    string getTex_d() const;
    void setTex_d(const string &value);
    string getTex_n() const;
    void setTex_n(const string &value);
    string getTex_s() const;
    void setTex_s(const string &value);
    // shader
    string getShader() const;
    void setShader(const string &value);

    // Save
    string toStringDiffuse();
    string toStringAmbiant();
    string toStringSpecular();

private:
    lsMaterial  *next_material;
    string      name;

    // Data
    float mdiffuse[4];
    float mambiant[4];
    float mspec[4];
    float transparent;
    float mshininess;
    float mglow;

    // textures
    string tex_d;
    string tex_n;
    string tex_s;
    // shader name
    string shader;

};

#endif /* LSMATERIAL_H_ */
